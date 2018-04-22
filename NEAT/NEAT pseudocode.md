﻿- Generation #0
  - population of 150 NEAT networks
  - **uniform** population of networks with zero hidden nodes

- Speciation
  - An ordered list of species is maintained.
  - In each generation, genomes are sequentially placed into species. 
  - Each existing species is represented by a random genome inside the species from the previous generation. 
    - A given genome g in the current generation is placed in the first species in which g is compatible with the representative genome of that species. 
    - speciate using a compatibility threshold δt versus compatibility distance δ
      - δt = 3.0
      - δ = c1 * E / N + c2 * D / N + c3 * W
        - E: number of excess genes
        - D: number of disjoint genes
        - W: average weight **differences** of matching genes, including disabled genes
        - c1, c2, c3: coefficients that allow us to adjust the importance of the three factors
          - c1 = 1.0, c2 = 1.0, and c3 = 0.4
        - N: the number of genes in the larger genome
          - N can be set to 1 if both genomes are small, i.e., consist of fewer than 20 genes.
  - If g is not compatible with any existing species, a new species is created with g as its representative.

Fitness
  - f(i) = potency of a run
    - 15 min?
    - Abort if too much nose-picking. Since i'm doing potency and not DPS, nose-picking appropriately penalizes fitness

Evaluation
  - All incoming connections use value from the **current**
    - Except recurrent (loops) which use **previous**
  - Sequentially check connections for loops
    - Add next connection to our temp array
    - If there is no loop, mark as non-recurrent, continue
    - If there is a loop, remove from temp array, mark as recurrent and continue
  - s_i = U * x_i + W * s_(i-1)
    - U is sparse matrix of weights of non-recurrent connections
    - W is sparse matrix of weights of recurrent connections
  - o_i = ϕ(V * s_i)
    - V is identity matrix to show only outputs
    - We used a modified sigmoidal transfer function, ϕ(x) = 1 / (1+e^(−4.9*x)) at all nodes. 
  
- Next generation
  - If the maximum fitness of a species did not improve in 15 generations, the networks in the stagnant species were not allowed to reproduce
  - The champion of each species with more than five networks was copied into the next generation unchanged
  - Every species is assigned a potentially different number of offspring in proportion to the sum of adjusted fitnesses f'(i) of its organisms.
    - f'(i) = f(i) / sigma(sh(δ(i, j)) where j is every organism in the **population**	
    - sh: sharing function is set to 0 when distance δ(i, j) is above the threshold δt; otherwise, sh(δ(i, j)) is set to 1
      - Thus, reduces to the number of organisms in the same species as organism i
      - This reduction is natural since species are already clustered by compatibility using the threshold δt.
  - Species then reproduce by first eliminating the lowest performing members from the population. 
  - The entire population is then replaced by the **offspring** of the remaining organisms in each species.
  - Crossover
    - In each generation, 25% of offspring resulted from mutation without crossover
    - The interspecies mating rate was 0.1%
    - genes in both genomes with same innovation numbers are lined up
    - disjoint: gene that does not match up, **within** range of other parent's innovation numbers
    - excess: gene that does not match up, **outside** range of other parent's innovation numbers
    - Offspring:
      - matching genes: randomly chosen from either parent
      - excess and disjoint genes: always included from the more fit parent
      - 75% chance that an inherited gene was disabled if it was disabled in either parent
  - Mutations
    - Connection weights perturbed
      - 80% chance of a genome having **all** its connection weights mutated
        - each weight had a 90% chance of being **uniformly** perturbed 
        - 10% chance of being assigned a new random value
    - add connection mutation
      - probability 5%
      - a single new connection gene with a random weight is added connecting two previously unconnected nodes. 
    - add node mutation
      - probability 3%
      - an existing connection is split and the new node placed where the old connection used to be. 
      - The old connection is disabled and two new connections are added to the genome. 
      - The new connection leading into the new node receives a weight of 1
      - the new connection leading out receives the same weight as the old connection
 