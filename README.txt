Lightwave 3D-mesh object format loading and parsing in C++.

Author: Ilkka Prusi
Contact: ilkka.prusi@gmail.com


Summary:
C++ classes to load and parse Lightwave objects for use in games (primarily).
Note that only newer "LWO2" format is properly documented,
older "LWOB" format handling may have bugs or other problems.

License:
MIT-style, see LICENSE.txt

Description:
Classes for loading 3D-mesh objects in Lightwave format to be used in games and such.

Started this project around 2004-2005 but has been waiting for time to finish
most of that time.. Started converting to use C++0x features but not finished.

Current Lightwave-SDK describes object format for LW 6.5 and newer ("LWO2").
Support for older formats (pre-6.0) is at least partially working.

Note that Lightwave seems to be moving towards common XML-based format
used by various other new software also so in future different handling will be necessary but that 
will be common for other software also.

Status:
"High-level" chunk-nodes in fileformat should be ~ready.
Lower-level sub-chunks have special cases which are not completed,
mostly shader-procs and textures are not completed.

Container of parsed information may need redesign for 
simpler information when used in apps.
Also texture-related handling is not completed.

Parts of code based on example code in Lightwave SDK.

Note that this is work in progress and not finished..

