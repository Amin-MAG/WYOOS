#ifndef __INTERUPTS_H
#define __INTERUPTS_H

#include "types.h"
#include "port.h"
#include "gdt.h"


class InterruptsManager
{
    public:
        InterruptsManager(GlobalDescriptorTable* gdt);
        ~InterruptsManager();

        static uint32_t handleInterrupt(uint8_t interrupt_number, uint32_t esp);
        
        static void ignoreInterruptRequest();
        static void handleInterruptRequest0x00();
        static void handleInterruptRequest0x01();
        void Activate();
    protected:
        struct GateDescriptor
        {
            uint16_t handlerAddressLowBits;
            uint16_t gdt_codeSegmentSelector;
            uint8_t reserved;
            uint8_t access;
            uint16_t handlerAddressHighBits;
        } __attribute__((packed));

        static GateDescriptor interruptDescriptorTable[256];

        struct InterruptDescriptorTablePointer
        {
            uint16_t size;
            uint32_t base;
        } __attribute__((packed));

        static void SetInterruptDescriptorTableEntry (
            uint8_t interruptNumber,
            uint16_t codeSegmentSelectorOffset,
            void (*handler)(),
            uint8_t DescriptorPrivilegeLevel,
            uint8_t DescriptorType
        );


};


#endif