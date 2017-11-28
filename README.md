# ETViewer
An alternative to Windows [TraceView](https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/traceview) utility.

## Description
This is a fork of ETViewer project on [codeplex](https://etviewer.codeplex.com/) maintained by
[Ellery_Pierce](https://www.codeplex.com/site/users/view/Ellery_Pierce) and
[jmartin](https://www.codeplex.com/site/users/view/jmartin).
Since codeplex is shutting down I forked
this project to preserve it for the open source community.

![ETViewer screenshot](http://download-codeplex.sec.s-msft.com/Download?ProjectName=etviewer&DownloadId=279342)

## System Requirements
Windows XP or higher.
The lastest dbghelp.dll available from [Debugging Tools for Windows](https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/)
package is needed when running under Windows XP.

## Features

Supported session types
* Full Real time tracing.
* Full log file support (.etl).

Trace provider management
* PDB based (no .tmf/.tmc support).
* Complete trace provider management (Activation, Flags and Levels).
* Multiple concurrent providers in the same session.
* Multiple providers in the same PDB.

Trace management
* Search (to find, delete or mark traces).
* Mark (Visual Studio style).
* Highlight (coloring traces, DebugView style).
* Instant Exclusion/Inclusion filters for Real-Time tracing.
* Column sorting.

Other features
* Source code visualization with simple sintax highlight.
* Drag&Drop support.
* Clipboard support.
* Export to text file.
* File association support (optional).

Tested platforms
* Windows XP 32 Bits
* Windows 2003 32 Bits
* Windows Vista 32 Bits
* Windows 7 64 Bits
* W2K not yet supported.

## Changelog

### Version 1.1

* Converted project to VS2015 with v140_XP and v140 toolsets
* Fixed bug with loading source files for viewing
* Made it so you can still drop .pdb files when running as admin when UAC is enabled
* Shortened the source file name field so you can see the filename
* Made it so special formatters (like !HRESULT!) will show their value
* Added trace example application that generates trace messages
* Moved around some of the folders and made it so everything builds to a bin folder that starts at the solution root

### Version 1.0

* Fixed issue with double %% in trace formatting.
* Added support for having multiple PDB files with same control GUID
* Now compiling with VS 2013 and have second target for XP compatibility.

### Version 0.9

* Fixed 64 bits PDB issues.
* File association support (optional).
* Command line arguments to open ETL, source and PDB files.
* Reload of PDB and source files (Auto / Ask / Disable).
* Configurable trace font.
* Configurable source file search paths.
* Settings dialog.
