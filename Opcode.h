#ifndef OPCODE_H
#define OPCODE_H

enum class Opcode {
    // Data Movement
    INCR = 1,       // Increment register
    ADDI,           // Add immediate value to register
    ADDR,           // Add two registers
    PUSHR,          // Push register to stack
    PUSHI,          // Push immediate value to stack
    MOVI,           // Move immediate value to register
    MOVR,           // Move register to register
    MOVMR,          // Move from memory to register
    MOVRM,          // Move from register to memory
    MOVMM,          // Move from memory to memory
    PRINTR,         // Print register value
    PRINTM,         // Print memory value
    PRINTCR,        // Print register as character
    PRINTCM,        // Print memory as character
    
    // Control Flow
    JMP,            // Unconditional jump
    JMPI,           // Jump immediate offset
    JMPA,           // Jump absolute address
    JMPE,           // Jump if equal (zero flag set)
    JMPNE,          // Jump if not equal
    JMPLT,          // Jump if less than
    JLT,            // Jump if less than (register offset)
    JLTI,           // Jump if less than (immediate offset)
    JLTA,           // Jump if less than (absolute address)
    JMPGT,          // Jump if greater than
    JGT,            // Jump if greater than (register offset)
    JGTI,           // Jump if greater than (immediate offset)
    JGTA,           // Jump if greater than (absolute address)
    JE,             // Jump if equal (register offset)
    JEI,            // Jump if equal (immediate offset)
    JEA,            // Jump if equal (absolute address)
    CALL,           // Call function
    CALLM,          // Call function with memory address
    RET,            // Return from function
    CMPI,           // Compare register with immediate
    CMPR,           // Compare two registers
    
    // Arithmetic
    SUB,            // Subtract
    MUL,            // Multiply
    DIV,            // Divide
    MOD,            // Modulo
    
    // Logical
    AND,            // Bitwise AND
    OR,             // Bitwise OR
    
    // Stack Operations
    POPR,           // Pop from stack to register
    POPM,           // Pop from stack to memory
    
    // System Calls
    ALLOC,          // Allocate memory
    FREE_MEMORY,    // Free allocated memory
    MAP_SHARED_MEM, // Map shared memory region
    SIGNAL_EVENT,   // Signal event (register)
    SIGNAL_EVENT_I, // Signal event (immediate)
    WAIT_EVENT,     // Wait for event (register)
    WAIT_EVENT_I,   // Wait for event (immediate)
    SLEEP,          // Sleep for N cycles
    SET_PRIORITY,   // Set process priority
    SETPRIORITY,    // Set process priority (register)
    SETPRIORITYI,   // Set process priority (immediate)
    EXIT,           // Exit process
    ACQUIRE_LOCK,   // Acquire lock (register)
    ACQUIRE_LOCK_I, // Acquire lock (immediate)
    RELEASE_LOCK,   // Release lock (register)
    RELEASE_LOCK_I, // Release lock (immediate)
    INPUT,          // Read input from console
    INPUTC,         // Read character from console
    TERMINATE_PROCESS, // Kill another process

    // Special
    NOOP = 0,       // No operation
    INVALID = 255   // Invalid opcode
};

#endif