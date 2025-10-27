# Serpent

Serpent is a C++ serialization and runtime memory layout library, designed for game engines.

## Overview

Every value in Serpent is defined by a Layout object, which defines the structure of the data.
Layouts define how the data is arranged in memory, allowing Serpent to calculate memory offsets and alignments of values at runtime.

Unlike dynamic layouts used in scripting languages like Javascript, Serpent enforces static, typed memory structures.
This is done for several reasons,
1. To reduce memory usage
    - Instead of having duplicated memory telling how an object is laid out, multiple objects can reference the same layout
2. To improve cache locality
    - Storing all the data for an object linearly reduces cache misses when accessing the same object
3. To minimize surface area for bugs
    - Weakly typed objects often defer mismatched types until far later than they occur.

## Use case

Serpent is intended to be used in scripting systems, as a means to share data between native code and scripting engines.
It can be used for various systems,
1. Entity data and components
2. Networking and packets
3. Vertex buffers and other graphics data
4. Save game data
5. An intermediate representation for config / data formats, like JSON, and native C++ structs 
