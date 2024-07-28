[Back to Readme](../README.md)

---

# Timings for Parallel PageRank Algorithm

### Here you can see the table of average iteration times with varying thread counts using Standard PageRank Algorithm.

| Threads | Avg Time (ms) |
|---------|---------------|
| 1       | 36223.1       |
| 2       | 18765.3       |
| 3       | 12953.7       |
| 4       | 10118.5       |
| 5       | 8200.73       |
| 6       | 7764.15       |
| 7       | 7210.95       |
| 8       | 6617.91       |
| 9       | 6384.74       |
| 10      | 7895.61       |

### This table shows the average time taken (in milliseconds) for iterations with each thread count. As the number of threads increases, the computation time generally decreases. However, at 10 threads, the time increases again, indicating potential overhead.

---

# Timings for montecarlo PageRank Algorithm

| Threads | Avg Time (ms) |
|---------|---------------|
| 1       | 63433.6       |
| 2       | 35355.3       |
| 3       | 25348.4       |
| 4       | 19916.5       |
| 5       | 16869.1       |
| 6       | 18315.1       |
| 7       | 19039.4       |
| 8       | 19224.7       |

### Same we can see here, from 6 threads the time starts to increase again, indicating overhead.

### Here you can see all configs for montecarlo algorithm.
```c++
int num_walks = 100000; // Number of random walks
int walk_length = 1000; // Length of each random walk
double damping_factor = 0.85; // Damping factor for PageRank (This specifies the probability of jumping to a random node)
int runs = 20; // Number of runs for averaging timings
int min_threads = 1; // Minimum number of threads to use
int max_threads = 10; // Maximum number of threads to use
```

---

# Comparison of Algorithms

### When comparing the performance of the PageRank and Monte Carlo algorithms, we can make following observations:
- **PageRank Algorithm:** Shows a consistent decrease in computation time with increasing up to 9 threads. Time increases at 10 threads, likely due to parallelism overhead.

- **Monte Carlo Algorithm:** Also demonstrates a decrease in computation time with more threads, but improvement is less pronounced compared to PageRank. An increase in time is seen starting from 6 thread suggesting potential overhead or diminishing returns.
### In summary, both algorithms demonstrate improved performance with more threads up to a certain point, but experience increased computation time at higher thread counts. The PageRank algorithm achieves better efficiency with larger number of threads compared to the Monte Carlo algorithm, indicating that it scales better with parallelism.