#include "interrupts.h"
#include "types.h"
#include "port.h"

void printf(char* str);

InterruptsManager::GateDescriptor InterruptsManager::interruptDescriptorTable[256];

void InterruptsManager::SetInterruptDescriptorTableEntry (
    uint8_t interruptNumber,
    uint16_t codeSegmentSelectorOffset,
    void (*handler)(),
    uint8_t DescriptorPrivilegeLevel,
    uint8_t DescriptorType
) {
    interruptDescriptorTable[interruptNumber].handlerAddressLowBits = ((uint32_t) handler) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].handlerAddressHighBits = (((uint32_t) handler) >> 16) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].gdt_codeSegmentSelector = codeSegmentSelectorOffset;
    const uint8_t IDT_DESC_PRESENT = 0x80;
    interruptDescriptorTable[interruptNumber].access = IDT_DESC_PRESENT | ((DescriptorPrivilegeLevel & 3) << 5) | DescriptorType;
    interruptDescriptorTable[interruptNumber].reserved = 0;
}

InterruptsManager::InterruptsManager(GlobalDescriptorTable* gdt){
    uint16_t code_segment = gdt->CodeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for(uint16_t i=0; i<256; i++) {
        SetInterruptDescriptorTableEntry(i, code_segment, &ignoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
    }

    SetInterruptDescriptorTableEntry(0x20, code_segment, &handleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, code_segment, &handleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);

    // // Setup PICs
    // SetupPics(
    //     pic_master_command,
    //     pic_master_data,
    //     pic_slave_command,
    //     pic_slave_data
    // );

    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor) -1;
    idt.base = (uint32_t) interruptDescriptorTable;
    asm volatile("lidt %0": : "m" (idt));
}

InterruptsManager::~InterruptsManager(){

}

void InterruptsManager::Activate(){
    asm("sti");
}

uint32_t InterruptsManager::handleInterrupt(uint8_t InterruptNumber, uint32_t esp){
    printf("Interupt !");

    return esp;
}