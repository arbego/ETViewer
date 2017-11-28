----------------------
----------------------
			
VERSION HISTORY
			
----------------------
----------------------
Version 1.1:
- Converted project to VS2015 with v140_XP and v140 toolsets
- Fixed bug with loading source files for viewing
- Made it so you can still drop .pdb files when running as admin when UAC is enabled
- Shortened the source file name field so you can see the filename
- Made it so special formatters (like !HRESULT!) will show their value
- Added trace example application that generates trace messages
- Moved around some of the folders and made it so everything builds to a bin folder that starts at the solution root

Version 1.0:
- Converted project to VS2013 with v120_XP and v120 toolsets
- Converted project to use unicode
- Fixed bug with handling %% in a format string
- String handling functions use safe versions
- Converted this file to english
- Added english string resources

-----------------------------------------
Forked from ETViewer 7/1/2014
-----------------------------------------

Version 0.9:

- Support recharge providers through contextual menu 
- Correction of% p in 64-bit PDBs 
- Configuration of policy files recharge (self, ask, disabled) 

Version 0.8.5: 

- Correction filter inclusion / exclusion loading 
- Correction to the problem of formatting I64x% e% I64x 
- Correction dialog flicker sources 

- Limitation to a single instance 
- File Associations (ETL, PDB, sources) 
- Support command line (/ etl / pdb / src / l / s) 
- Dialog settings 
- Alternative sources directories 
- Source panel traces 
- File Associations 

Version 0.8: 

- Correction of synchronization between the two viewfinders highlight filters. 
- Support for opening etl logs format. 
- Support for creating logs etl format. 

Version 0.6: 

- Support for multiple providers for PDB. 
- Ordination by columns, the default is ascending ordination by index (should preserve the order of events). 
- Ordination stable. It should retain the last items ordination in the same order in the new ordination. 
- Improved management of TimeStamps. The most accurate timestamp available in the implementation of eventracing platform is used. 
- Improved visualization of events (flicker removed). 
- Persistence basic (global, not workspaces): 
- Filters inclusion / exclusion. 
- Filters Highlight. 
- Recent PDBs open. 
- Recent open sources. 
- Columns trace viewer. 
- Font size of the trace viewer. 
- State the last viewing event. 

Version 0.5: 

- Comprehensive Management of suppliers (Activation, and Trace Flag Trace Level) 
- Display of real-time trace multiple suppliers 
- Visualization of sources 
- Visualization of sources from the trace 
- Searches for text trace 
- Deleting text traces per trace 
- Marked trace style visual studio (F2, Ctrl + F2, Shift + Ctrl + F2) 
- Filters rapid inclusion / exclusion by text traces. 
- Filters highlight trace 
- Support Drag & Drop 
- Support for exporting to txt traces.. 
- Support for copying trace the ClipBoard.