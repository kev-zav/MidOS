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
    
    // Control Flow
    JMP,            // Unconditional jump
    JMPE,           // Jump if equal (zero flag set)
    JMPNE,          // Jump if not equal
    JMPLT,          // Jump if less than
    JMPGT,          // Jump if greater than
    CALL,           // Call function
    RET,            // Return from function
    CMP,            // Compare two values
    
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
    MAP_SHARED_MEM, // Map shared memory
    SIGNAL_EVENT,   // Signal an event
    WAIT_EVENT,     // Wait for an event
    SLEEP,          // Sleep for N cycles
    SET_PRIORITY,   // Set process priority
    EXIT,           // Exit process
    ACQUIRE_LOCK,   // Acquire a lock
    RELEASE_LOCK,   // Release a lock
    INPUT,          // Read input from console
    TERMINATE_PROCESS, // Kill another process
    
    // Special
    NOOP = 0,       // No operation
    INVALID = 255   // Invalid opcode
};

#endif