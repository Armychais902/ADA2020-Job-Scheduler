# ADA2020 Job Scheduler
Algorithm Design and Analysis, Fall 2020, Project - Job Scheduler

## Introduction
This is the final project of ADA lecture in NTU CSIE:

>The NTU CSIE NASA Workstation Team is thinking of developing an algorithm to schedule tasks on the GPU servers.
(Disclaimer: The scenario is purely hypothetical and does not reflect real-world decisions.)

>For simplicity, we assume that students submit jobs to run before a given point of time (say, every midnight).
In other words, the scheduling can be done offline.

>We also assume that the students can predict to a certain extent how long their jobs will run.
(For example, we can kill tasks that are over the time limit, while using the “bubbles” caused by tasks finishing before expectation for non-scheduled jobs.)

>The goal is to minimize:
>- The *makespan*, i.e., the total time it takes to finish the jobs
>- The *weighted total flow time*, i.e., the weighted sum of the finishing times

>--<cite>[ Final Challenge][1]</cite>

[1]: https://hackmd.io/6zxbedYCSLe8M8hW1Su0ww

### Definitions
**Definition of terms is from <cite>[ Final Challenge][1]</cite>**

[1]: https://hackmd.io/6zxbedYCSLe8M8hW1Su0ww**

#### *Slice*
Can be thought of units of computational power (e.g., VRAM.).

#### *Job*
Contains some *operations*. The operations in a job may depend on each other in some order.

#### *Operation*
A basic task that requires some slices of computing resources and a *duration* to run. Cannot be preempted.

#### *Duration*
The time required to execute an operation.

#### *Makespan*
The total time required to finish the jobs. ([Wikipedia](https://en.wikipedia.org/wiki/Makespan))

#### *Finishing Time*
The time when every operation in a job is finished.

#### *Weighted Total Flow Time*
The weighted sum of the finishing time for each job, i.e., $\sum_i w_i C_i$, where $w_i$ and $C_i$ are the weight and finishing time of the $i$-th job respectively

### Metric to Optimize
The sum of the makespan and the weighted total flow time.

## How to Run

## Contributions
- 李英華(Me)
  
  - Merged methods and unified code from teammates; rebuilt the scheduling criteria from a job-based perspective.
  
  - Generated private test cases.
  
  - `./scheduler_merge.cpp`
  
  - `./test/team17-test-cases`

- 陳詩淇
  
  - Coded up the basis of our program, optimize public test cases 00,02.
  
  - `./src/challenge_origin.cpp`

- 林芸平
  
  - Implemented the linked list which keeps track of the empty time slots, optimize the rest of the public test cases.
  
  - `./src/challenge.cpp`

- 戴可葳
  
  - Implemented different scheduling criteria.
  
  - `./src/slice_swap.cpp`

- 張嘉淳

  - Implemented the scheduler based on meta-heuristic algorithm.

## Resources
https://hackmd.io/6zxbedYCSLe8M8hW1Su0ww?both
