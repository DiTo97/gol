# Vectorization with icc

### 1. No vectorization: icc -O0  or icc -O1
### 2. 

To see if the loop was vectorize we use the `-vec-report` or `-qopt-report=2 -quopt-report-phase=vect`