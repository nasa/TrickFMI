# TrickFMI: A Functional Mockup Interface (FMI) Standard Implementation for Trick Base Models and Simulations

## Brief Abstract:

This software supports FMI based model exchange with Trick based simulations.

Simulation is a key technology used in the conception, design, development and operation of human space systems.  As these space system become more and more complex, so do the models used in the simulations of these systems.  This software provides a practical method for exchanging models between NASA, its contractors and its international partners.

The Functional Mockup Interface (FMI) standard was developed in partnership with governmental, academic and commercial entities in the European Union.  This standard is used to support the exchange of component models for complex system simulations throughout Europe and the United States.  Trick simulations are used all across NASA for simulations that support human spaceflight activities.  However, until now, there were no means to use FMI based models in a Trick based simulation or a method for providing Trick based models that were FMI compliant.  This software provides implementation software to do both.

There are two principal components to the software:
- A C based software implementation for wrapping Trick based C models that provide an FMI compliant interface;
- A collection of C++ classes that can be used in a Trick based simulation to use an FMI compliant model (FMU).

## Copyright:
Copyright 2017 United States Government as represented by the Administrator of the National Aeronautics and Space Administration.  No copyright is claimed in the United States under Title 17, U.S. Code. All Other Rights Reserved.

