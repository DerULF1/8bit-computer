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

## Short Descriptions of the Modules
The majority of the chips are from the 74HCT series so that they are fully compatible with the 74LS series but use much less power. Exceptions are the 74HC595 (shift register) and the 74LS382 (ALU chip) which are not available in a 74HCT version and the 74HC14 (not gate with Schmitt trigger) that was explicitly used because of the more even contribution between low level and high level voltage whenever I suspected a noisy signal.

### [Clock](Schematics/Clock.pdf)
The clock module is very similar to Ben Eater's clock module. The major change is that the halt line is stopping only the clock signal from the first 555 timer. With that change it is possible to manually step over a HALT statement in the code.

Another change was to invert the generated clock signal so that the duty cycle becomes less than 50%. My calculations were showing a longer overall switching time during the low clock than for the high clock signal.

### [ALU](Schematics/ALU.pdf)
My ALU is based on the features of the 74LS382 chip that provides addition and subtraction as well as logical and, logical or and logical xor. It's also possible to have the fixed values 0x00 or 0xff as output. Two shift operations (right shift or divide by 2 and left shift or multiply by 2) are implemented as explicit wiring of the input signals. The output is selected by a 74HCT153 based on the state of the control lines.

The ALU has an additional internal register which can either be loaded from the bus or with the output from the calculation. There is a control line that shows the content of the internal register to the bus. In all calculations with two operands the first operand is taken from the data bus and the second operand is taken from the internal register. Calculations with only one operand take that operand from the bus.

All arithmetic and logical functions of the ALU are chosen with four dedicated control lines.   

Another part of the ALU is the flag register that holds the following flags:
* carry flag
* overflow flag
* negative flag
* zero flag

While the carry flag and the overflow flag are taken as output from the 74LS382, the zero flag is calculated separately and the negative flag is always just bit 7 of the last result. During shift operations the carry flag is taken from the bit that was shifted out.

There is a control line that shows the 4 bit flag register to the bus. It's also possible to read the flag register from the bus. That allows the flag register to be put on and fetched from the stack.

### [Control Logic](Schematics/ControlLogic.pdf) / [Instruction Decoder](Schematics/InstructionDecoder.pdf)
One of the most important changes to Ben Eater's design was to take away the instruction register from the address lines of the micro code EEPROMs. Instead of the four bit step counter in the original design I use a twelve bit micro code program counter.

The begin of the micro code for each instruction is read from another set of EEPROMs based on the content of the instruction register and the flags register. At the end of the micro code for an instruction there is a dedicated control signal that resets the micro code counter to the start of the instruction fetch routine.

With that design the address lines on the micro code EEPROMs only change at clock low. So the control lines only change during the clock low phase. Another advantage is that there's no fixed number of micro code steps per instruction. There are no wasted bytes in the EEPROM after an instruction reset (and not even empty cycles for unused bytes) and there is also no upper limit (but the size of the micro code counter itself) for the number of micro code steps for an instruction. That setup also allows to do conditional and unconditional jumps in the micro code.

It also allows to integrate the interrupt feature this way. This is done by an additional address line for the instruction decoder, which is high whenever an interrupt is requested and not yet inhibited. With this address line the start of the fetch routine is rerouted to the start of the interrupt micro code. That code will save the current program counter and the flags register on stack and load the program counter with the user defined interrupt handler address. 

I also use the micro code to do the initial setup of the system at reset time. So the only important reset signal goes to the micro code counter and forces the system to start at micro code position zero. The micro code here is doing all the initialization of the other registers, sets the start address for the program counter and then hands over to the fetch routine to start normal program execution.

A special case for the control logic is the write access to the data bus. Because only one register at a time is allowed to have write access to the data bus I use a 4-to-16 demultiplexer to save control lines and make sure there's only one writer enabled. 

### [Memory](Schematics/memory.pdf)
In my design there is no single dedicated memory address register but three different 16 bit registers are available which allow to define the actual memory address to be used. Those three registers are
* [Program Counter](Schematics/ProgramCounter.pdf)
* [Stack Address](Schematics/StackAddress.pdf)
* [New Address](Schematics/NewAddress.pdf)

All three registers have direct connection to the address line of the RAM chip and some care must be taken in the micro code to only have one of those registers having the output to the address bus enabled at one step.

The program counter and the stack address are special purpose registers whereas the new address register is used whenever there is a memory address to be used that is put together by two 8 bit numbers coming from the data bus. All address registers have control lines that allow them to put their upper or lower byte value to the address bus. But only the new address register is also able to read single bytes from the data bus. The other two registers read their new value from address bus which is defined by the content of the new address register.

### [Port Selector](Schematics/PortSelector.pdf)
To add external devices to the machine I use an 8 bit port mechanism. That mechanism allows external controllers to read from or write to the data bus. Two dedicated port instructions (INP, OUT) move that byte from the bus to the A register or vice versa. That way it is possible to add devices without the need to change the micro code.

Each controller must assure that it doesn't write to the data bus as long as it isn't told to do so. 

## External Devices
Since each external device needs its own specialized decoder I built decoders for those devices I wanted to be included.

### [LCD Display](Schematics/LCD.pdf)
The 4x20 characters display I connected has a 4 bit communication mode which allows to completely control the display with only seven lines. In that mode each byte send to or received from the display must be split into two half bytes and be transmitted one after the other leading to more complexity on the software side. But on the other hand that way the display only uses one port.

As there is one bit left over which is not needed to talk to the display I use that bit to control the power of the back light of the display.

### [SPI Bus](Schematics/SDCard.pdf)
Since the SPI bus only uses two output control lines and one input control line it only needs two chips to set up an SPI controller. One chip (74HCT245) is the bus transceiver the other (74HCT173) is holding the two output bits to the SPI bus. The two other bits of that chip are used to select the device on the SPI bus.

I have only one device on the SPI bus yet (an SD card board) so I use the last bit of the 173er to switch the power supply for the SD card.

### [PS/2 Keyboard](Schematics/PS2Controler.pdf)
The PS/2 controller is built to receive a single scan code from the keyboard asynchronously ignoring the parity bit. A byte from the keyboard may be received at any time and is completely independent of the speed that he computer is running on. Whenever a byte is received the controller sets an interrupt request and inhibits the keyboard from sending further bytes until the last byte is read from the buffer. That way the computer has all the time it needs to receive and interpret one byte after the other.

It's also possible to send bytes to the keyboard by giving full control over the two control lines on the PS/2 bus. The PS/2 protocol must be implemented on the software side. The implementation is quite timing critical, because the keyboard is sending the clock signal and expects the computer to respond as fast as the clock ticks which is around 10kHz.