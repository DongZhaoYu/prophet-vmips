prophet-vmips
=============

prophet-vmips is based on the vmips simulator and it can support the speculative execution model which is developed in paper "Prophet: A Speculative Multi-threading Execution Model with Architectural Support Based on CMP". The prophet model aims to explore parallelism from sequential programs by speculative execution. The program source code first needs be compiled to executable program with the prophet compiler. Then it can be run on the prophet-vmips simulator.

The instruction set supported by prophet-vmips is an augmented version of the MIPSR3000 instructions. Those new instructions are added to support speculation: fst, ust, spawn, squash, cqip, pslice_entry and pslice_exit.

