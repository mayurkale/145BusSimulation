145BusSimulation
================

Emulation for PMU-LPDC-SPDC communication in SmartGrid system

Mayur:
Below functionality/code was added to Swadesh's code for 145 Bus Simulation
1) Application7() : This application allows DQT approach to work as CUT in case of an application that requires all the data to be sent to SPDC 
   for offline analysis.
   For application7() below are major code changes:
   i)  Added time_alignk(),intermediate_dispatchk(),dispatchk(),craetedataframek(),assign_df_to_TSBk() functions to replicate functionality of           CUT approach in which, dispatch functions aggregates all the frames of one stamp (from PMUs) and sends one large frame to SPDC.
   ii) Wrote Analysing function cases 70,71,72,73,73 for calculating min. pmu to LPDC latency (case 70,71), frame creation time in dispatch (case        74),min LPDC to SPDC latency (case 72,73) 
   
