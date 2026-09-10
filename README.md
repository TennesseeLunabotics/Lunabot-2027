# Lunabot-2027
Lunabotics repository for the 2026-2027 NASA Lunabotic compition.

All code added to this Github needs to pass these requirements:

---

SSTA Software Manifesto
Guiding principles to writing mission-critical software.
--
## Universal Principles
- Prefer strongly typed languages.
- Configuration is king.
- Check every incoming argument and return value.
- Declare every data object at the smallest possible scope.
- Never fail silently. Errors must bubble up until they are explicitly handled.
- Benchmark critical paths. Always know how long functions and data flows take to complete.
- Design the system (architecture, data flow, interfaces) before writing code.
- Get the tooling, build system, and development environment working before writing code.
- Write descriptive commit messages.
- Write clear, useful comments. Comments should describe what you are doing, not how you are doing it.
- Pick the write tool (language, framework, design principle) for the job.

## Embedded Firmware (C/C++)
- No dynamic memory allocation.
- Give all loops a fixed upper bound (except the main control loop).
- Enable maximum compiler warnings and treat warnings as errors.
- No recursion.
- Keep Interrupt Service Routines (ISR) extremely concise and avoid nested function calls.
- No function pointers.
- Global state must live inside well-defined structs.
- Minimize external libraries. Only include the exact headers you need.
- No compiler-magic or undefined behavior in core logic.
- You must be able to explain every line of mission-critical code.
- Unit-test all core logic.

## Non-Embedded Language Specific Guidelines
## C++
- Leverage the Standard Template Library (STL) when possible.

## Python
- Use Python when it is the right tool. Especially when leveraging mature open-source libraries.
- Use type hints + a type checker.

## JavaScript / TypeScript
- Prefer TypeScript.
- Break all UI into digestible, reusable components.
