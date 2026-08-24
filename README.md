 Multithreaded Magic Square Validator (MMSV)
 
A C program that validates whether an `n x n` matrix is a magic square, using four concurrent worker threads to check rows, columns, diagonals, and number uniqueness in parallel — with a mutex-protected shared score to safely track results across threads.
 
Built for Operating Systems (COMP2006) at Curtin University Colombo, April 2026, focused on hands-on multithreading, shared-resource synchronization, and mutex-based critical sections.
 
## What it does
 
Given a matrix in a text file, the program spawns four threads:
 
- **Row thread** — checks every row sums to the magic constant
- **Column thread** — checks every column sums to the magic constant
- **Diagonal thread** — checks both the main and secondary diagonal
- **Uniqueness thread** — checks the grid contains every integer from `1` to `n²` exactly once
Each thread updates a shared `score` variable as it validates each component (+1 for a pass, -1 for a fail, floored at 0). Since all four threads can hit `score` at the same time, every update is wrapped in a mutex lock so increments and decrements never overlap or get lost. The main thread only reads the file, spawns the workers, waits for them to finish, and prints the final report — it does no validation itself.
 
## Build & Run
 
```bash
gcc -o mmsv mmsv.c -lpthread
./mmsv input.txt
```
 
**Input format:** first line is `n`, followed by the `n x n` matrix.
 
```
3
8 1 6
3 5 7
4 9 2
```
 
## Sample Output
 
```
Thread ID-3: Diagonal checks completed.
Thread ID-4: Uniqueness check completed.
Thread ID-2: Column checks completed.
Thread ID-1: Row checks completed.
 
--- Magic Square Report ---
Rows: All Valid
Cols: All Valid
Diags: All Valid
Unique: Passed
Final Score: 9 / 9
RESULT: VALID MAGIC SQUARE
```
