Ryan Tempas and Jon Bauer

Grid Layout
===========
   [2]
[1] 0 [3]
   [4]

Logging
=======
Set loggingRank in main.cpp to log a specific grid. defualts to grid 1.


Timing on HPCC
==============

Grid Size  |   Cars in Grid   |   Timesteps  |  Time in micro seconds
=====================================================================
    5                1               10               610
    5                4               10               632
    5                4               100              9149
    10               5               100              8514
    10               9               100              8225
    10               9               1000             79628    
    10               1               1000             61067
    100              10              10000            4633448
    100              90              10000            4712148
    100              15              1000000          891953840
