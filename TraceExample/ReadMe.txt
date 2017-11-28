----------------------
----------------------
			
Trace Example Program
			
----------------------
----------------------

This program utilizes the wpp.targets file included with this project as well as the files in the WppDDKFiles folder to
create trace format information in the project. The technique used here is from http://devproconnections.com/visual-studio-2010/wpp-tracing-visual-c-2010-projects

The .targets file is added to this project via the following statement in the project file.

<ImportGroup Label="ExtensionTargets">
  <Import Project="Wpp.targets" />
</ImportGroup>

In the .targets file there are a couple of paths to tools and the \dev1 settings files. If you have a problem with building
this project it is most likely because the paths aren't pointing to their intended program or settings files.

Another issue that I've seen and is mentioned in the comments on the original article page is that if you compile with 
edit and continue enabled /ZI instead of /Zi you get an error where it can't find the function name of the trace function.