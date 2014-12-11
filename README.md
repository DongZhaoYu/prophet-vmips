prophet-vmips
=============

prophet-vmips is based on the vmips simulator and it can support the speculative execution model which is developed in paper "Prophet: A Speculative Multi-threading Execution Model with Architectural Support Based on CMP". The prophet model aims to explore parallelism from sequential programs by speculative execution. To run a program on prophet-vmips, the program source code needs be compiled to executable binary with the prophet tool chain.

The instruction set supported by prophet-vmips is an augmented version of the MIPSR3000 instructions. Those new instructions are added to support speculation: fst, ust, spawn, squash, cqip, pslice_entry and pslice_exit.

