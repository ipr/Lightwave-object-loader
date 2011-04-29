
Lightwave 3D-mesh object format loading and parsing in C++.

Author: Ilkka Prusi
Contact: ilkka.prusi@gmail.com


Summary:
C++ classes to load and parse Lightwave objects for use in games (primarily).

Description:
Classes for loading 3D-mesh objects in Lightwave format to be used in games and such.
Started this around 2006/2007 but has been on hold most of the time..

Current Lightwave-SDK describes object format for LW 6.5 and newer.
Support for older formats (pre-6.0) is at least partially working.

Status:
"High-level" chunk-nodes in fileformat should be ready.
Lower-level sub-chunks have special cases which are not completed,
primarily shader-procs are not completed.

Container of parsed information may need redesign for 
simpler information when used in apps.
Also texture-related handling is not completed.

Parts of code based on example code in Lightwave SDK.

