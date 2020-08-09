# 8bit-computer
## Introduction
Like some more people I was highly impressed by [Ben Eater's series](https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU) about building a computer from simple ICs on breadboards. My first attempts to work on electronics decades ago all failed because of my disability to solder. So building without soldering was what caught my attention.
My personal background on computers is a degree in applied computer science but I have no deeper knowledge in electronics except for the basics they teach in school. But [James Bates' series](https://www.youtube.com/playlist?list=PL_i7PfWMNYobSPpg1_voiDe6qBcjvuVui) convinced me to try some enhancements on Ben Eater's additional design.

## Major Enhancements to Ben Eater's Design
* [16-bit address bus with 64k bytes of RAM](Schematics/memory.pdf)
* [four general purpose registers](Schematics/Register.pdf)
* [ALU with boolean and shift operations](Schematics/ALU.pdf)
* [dividing instruction decoding](Schematics/InstructionDecoder.pdf) and [control logic](Schematics/ControlLogic.pdf)
* [four 8-bit digital input-/output-ports](Schematics/PortSelector.pdf)
* [16-bit stack pointer](Schematics/StackAddress.pdf)
* [interrupt controller](Schematics/InstructionDecoder.pdf)

## Connected Peripheral Devices
* [20x4 LCD display](Schematics/LCD.pdf) 
* [SD Card reader/writer](Schematics/SDCard.pdf)
* [PS/2 keyboard](Schematics/PS2Controler.pdf)