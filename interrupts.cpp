#include "interrupts.h"
#include "types.h"
#include "port.h"

#define NULL nullptr

void printf(char* str);

InterruptsManager::GateDescriptor InterruptsManager::interruptDescriptorTable[256];

InterruptsManager* InterruptsManager::ActiveInterruptManager = 0;


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

InterruptsManager::InterruptsManager(GlobalDescriptorTable* gdt) :  picMasterCommand(0x20),
                                                                    picMasterData(0x21),
                                                                    picSlaveCommand(0xA0),
                                                                    picSlaveData(0xA1){
    uint16_t code_segment = gdt->CodeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE = 0xE;

    for(uint16_t i=0; i<256; i++) {
        handlers[i] = NULL;
        SetInterruptDescriptorTableEntry(i, code_segment, &ignoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
    }

    SetInterruptDescriptorTableEntry(0x20, code_segment, &handleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, code_segment, &handleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);

    picMasterCommand.Write(0x11);
    picSlaveCommand.Write(0x11);

    picMasterData.Write(0x20);
    picSlaveData.Write(0x28);

    picMasterData.Write(0x04);
    picSlaveData.Write(0x02);

    picMasterData.Write(0x01);
    picSlaveData.Write(0x01);

    picMasterData.Write(0x00);
    picSlaveData.Write(0x00);

    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor) -1;
    idt.base = (uint32_t) interruptDescriptorTable;
    asm volatile("lidt %0": : "m" (idt));
}

InterruptsManager::~InterruptsManager(){
}

void PrintUnhandledInterrupt(uint8_t interrupt_number) {
	const char* hex_digits = "0123456789ABCDEF";
	char* message = "Unhandled interrupt: 0x00\n\n\n";
	message[23] = hex_digits[(interrupt_number >> 4) & 0x0F];
	message[24] = hex_digits[interrupt_number & 0x0F];
	printf(message);
}


uint32_t InterruptsManager::HandleInterruptNonStatic(uint8_t interrupt_number, uint32_t esp) {

	if(handlers[interrupt_number] != NULL) {
		esp = handlers[interrupt_number]->HandleInterrupt(esp);
	} else {
		PrintUnhandledInterrupt(interrupt_number);
	}

	if(interrupt_number >= 0x20 && interrupt_number <= 0x2f){
		picMasterCommand.Write(0x20);
		if(0x28 <= interrupt_number) {
			picSlaveData.Write(0x20);
		}
	}
	return esp;
}


void InterruptsManager::Activate() {
	if(ActiveInterruptManager != this) {
		ActiveInterruptManager->Deactivate();
	}
	ActiveInterruptManager = this;
	asm("sti");
}

void InterruptsManager::Deactivate(){
	if(ActiveInterruptManager == this) {
		ActiveInterruptManager = 0;
		asm("cli");
	}
}

uint32_t InterruptsManager::handleInterrupt(uint8_t interrupt_number, uint32_t esp) {
	if(ActiveInterruptManager != 0)
	{
		return ActiveInterruptManager->HandleInterruptNonStatic(interrupt_number, esp);
	}
	return esp;
}



InterruptHandler::InterruptHandler(uint8_t interrupt_number, InterruptsManager* interrupt_manager)
{
	this->interrupt_number_ = interrupt_number;
	this->interrupt_manager_ = interrupt_manager;
	this->interrupt_manager_->handlers[this->interrupt_number_] = this;
}
InterruptHandler::~InterruptHandler(){
	if(this->interrupt_manager_->handlers[this->interrupt_number_] == this) {
		this->interrupt_manager_->handlers[this->interrupt_number_] = NULL;
	}
}
uint32_t InterruptHandler::HandleInterrupt(uint32_t esp){
	return esp;
}