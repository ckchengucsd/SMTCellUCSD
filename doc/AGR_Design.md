# Gear Ratio Design
This is a documentation regarding the various design elements inside Gear Ratio (GR).

## Vertex Generation
Vertex Generation requires two steps in GR design: (1) Grid Graph Initialization (2) Vertex Initialization. 

In (1) Grid Graph Initialization step, we determine the standard cell width and height after scaled by the corresponding metal pitch distance. Then, we "jump" the graph by its relevant metal pitch distance. When traversing through middle layers, we always traverse twice since in worst case scenario, we have two sets of vertices induced by the two adjacent metal layers. Finally, this step will return all the useful mapping of columns, rows with their metal layers. These mappings will be repeatedly used for downstream tasks. 

Let us take a simple M2 Grid Graph for example, below is the vertex we want to retrieve. $\epsilon$ represents the offsets (for demo purpose, here we have different offsets $\epsilon_1$ and $\epsilon_3$). $MP_1$ and $MP_3$ are the metal pitches corresponding to metal 1 (`M1`) and metal 3 (`M3`).

<p align="center">
    <img src="/doc/figure/M2Grid.png" width="650">
</p>

However, without traversing through the grid, we have no idea on how many columns we have for M2. Also, we need to disregard any columns that are overlapped between `M1` and `M3`.

To think this more intuitively, we simply traverse the graph TWICE and merge columns if necessary.
In the first step, we gather all the columns induced by `M3`.

<p align="center">
    <img src="/doc/figure/Step1.png" width="650">
</p>

In the second step, we gather all the columns induced by `M1`. If there is any overlapping columns, we simply ignore them.

<p align="center">
    <img src="/doc/figure/Step2.png" width="650">
</p>

In (2) Vertex Initialization step, we use the mappings described above to generate all the vertices. Each vertex are defined under the name `m#r#c#`, where each `#` is a number corresponding to their metal, row and column information. Alongside these definition, we also associate them with there adjacent neighbor vertices (Directions: _L_, _R_, _F_, _B_, _U_, _D_, _FL_, _FR_, _BL_, _BR_). Since we already have all the column, row and metal information. This step will intuitive and computationally cheap.