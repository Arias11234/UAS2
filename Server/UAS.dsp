# Microsoft Developer Studio Project File - Name="UAS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=UAS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "UAS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UAS.mak" CFG="UAS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UAS - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "UAS - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "UAS"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "UAS - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 .\Release\CRCWheel.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /out:".\Release\Server.exe"

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /GX /ZI /Od /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\Debug\CRCWheel.lib comctl32.lib winmm.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"..\Debug\Server.exe" /pdbtype:sept
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ENDIF 

# Begin Target

# Name "UAS - Win32 Release"
# Name "UAS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Item Objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cAmmo.cpp
# End Source File
# Begin Source File

SOURCE=.\cArmor.cpp
# End Source File
# Begin Source File

SOURCE=.\cBooks.cpp
# End Source File
# Begin Source File

SOURCE=.\cClothes.cpp
# End Source File
# Begin Source File

SOURCE=.\cFoci.cpp
# End Source File
# Begin Source File

SOURCE=.\cFood.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cGems.cpp
# End Source File
# Begin Source File

SOURCE=.\cHealingCon.cpp
# End Source File
# Begin Source File

SOURCE=.\cHealingKits.cpp
# End Source File
# Begin Source File

SOURCE=.\cJewelry.cpp
# End Source File
# Begin Source File

SOURCE=.\cLockpicks.cpp
# End Source File
# Begin Source File

SOURCE=.\cManaStones.cpp
# End Source File
# Begin Source File

SOURCE=.\cMisc.cpp
# End Source File
# Begin Source File

SOURCE=.\cPack.cpp
# End Source File
# Begin Source File

SOURCE=.\cPlants.cpp
# End Source File
# Begin Source File

SOURCE=.\cPyreals.cpp
# End Source File
# Begin Source File

SOURCE=.\cSalvage.cpp
# End Source File
# Begin Source File

SOURCE=.\cScrolls.cpp
# End Source File
# Begin Source File

SOURCE=.\cShield.cpp
# End Source File
# Begin Source File

SOURCE=.\cSpellComponents.cpp
# End Source File
# Begin Source File

SOURCE=.\cTradeNotes.cpp
# End Source File
# Begin Source File

SOURCE=.\cTradeSkillMats.cpp
# End Source File
# Begin Source File

SOURCE=.\cWand.cpp
# End Source File
# Begin Source File

SOURCE=.\cWeapon.cpp
# End Source File
# End Group
# Begin Group "World Objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cAltar.cpp
# End Source File
# Begin Source File

SOURCE=.\cChest.cpp
# End Source File
# Begin Source File

SOURCE=.\cCovenantCrystal.cpp
# End Source File
# Begin Source File

SOURCE=.\cDoor.cpp
# End Source File
# Begin Source File

SOURCE=.\cHooks.cpp
# End Source File
# Begin Source File

SOURCE=.\cHouse.cpp
# End Source File
# Begin Source File

SOURCE=.\cLifestone.cpp
# End Source File
# Begin Source File

SOURCE=.\cMerchantSign.cpp
# End Source File
# Begin Source File

SOURCE=.\cPortal.cpp
# End Source File
# Begin Source File

SOURCE=.\cStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\cWorldObject.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Allegiance.cpp
# End Source File
# Begin Source File

SOURCE=.\Avatar.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CharacterServer.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cItemModels.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Client.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cMagicModels.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cModels.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cMonsterServer.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cNPC.cpp
# End Source File
# Begin Source File

SOURCE=.\cNPCModels.cpp
# End Source File
# Begin Source File

SOURCE=.\CommandParser.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Corpse.cpp
# End Source File
# Begin Source File

SOURCE=.\CorpseCleaner.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cPhysics.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cSpell.cpp
# End Source File
# Begin Source File

SOURCE=.\cWObjectModels.cpp
# End Source File
# Begin Source File

SOURCE=.\Database.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DatFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Fellowship.cpp
# End Source File
# Begin Source File

SOURCE=.\Job.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MagicSystem.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MasterServer.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Message.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Monster.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Object.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SimpleAI.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Status.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TreasureGen.cpp
# End Source File
# Begin Source File

SOURCE=.\WarSpell.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WinMain.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WorldManager.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WorldServer.cpp

!IF  "$(CFG)" == "UAS - Win32 Release"

# ADD CPP /Zi /I "C:\Program Files\Microsoft Visual Studio\VC98\INCLUDE"
# SUBTRACT CPP /O<none>

!ELSEIF  "$(CFG)" == "UAS - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Account.h
# End Source File
# Begin Source File

SOURCE=.\Allegiance.h
# End Source File
# Begin Source File

SOURCE=.\Avatar.h
# End Source File
# Begin Source File

SOURCE=.\CannedPackets.h
# End Source File
# Begin Source File

SOURCE=.\CharacterServer.h
# End Source File
# Begin Source File

SOURCE=.\cItemModels.h
# End Source File
# Begin Source File

SOURCE=.\Client.h
# End Source File
# Begin Source File

SOURCE=.\cMagicModels.h
# End Source File
# Begin Source File

SOURCE=.\cModels.h
# End Source File
# Begin Source File

SOURCE=.\cMonsterServer.h
# End Source File
# Begin Source File

SOURCE=.\cNPCModels.h
# End Source File
# Begin Source File

SOURCE=.\CommandParser.h
# End Source File
# Begin Source File

SOURCE=.\CorpseCleaner.h
# End Source File
# Begin Source File

SOURCE=.\cPhysics.h
# End Source File
# Begin Source File

SOURCE=.\cSpell.h
# End Source File
# Begin Source File

SOURCE=.\cWObjectModels.h
# End Source File
# Begin Source File

SOURCE=.\Database.h
# End Source File
# Begin Source File

SOURCE=.\DatFile.h
# End Source File
# Begin Source File

SOURCE=.\events.h
# End Source File
# Begin Source File

SOURCE=.\Fellowship.h
# End Source File
# Begin Source File

SOURCE=.\Job.h
# End Source File
# Begin Source File

SOURCE=.\MagicSystem.h
# End Source File
# Begin Source File

SOURCE=.\mainpage.h
# End Source File
# Begin Source File

SOURCE=.\MasterServer.h
# End Source File
# Begin Source File

SOURCE=.\Message.h
# End Source File
# Begin Source File

SOURCE=.\Object.h
# End Source File
# Begin Source File

SOURCE=.\PacketPipe.h
# End Source File
# Begin Source File

SOURCE=.\RecvPacket.h
# End Source File
# Begin Source File

SOURCE=.\Shared.h
# End Source File
# Begin Source File

SOURCE=.\SimpleAI.h
# End Source File
# Begin Source File

SOURCE=.\Status.h
# End Source File
# Begin Source File

SOURCE=.\TreasureGen.h
# End Source File
# Begin Source File

SOURCE=.\uas.h
# End Source File
# Begin Source File

SOURCE=.\VersionNo.h
# End Source File
# Begin Source File

SOURCE=.\WorldManager.h
# End Source File
# Begin Source File

SOURCE=.\WorldServer.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\UAS.ico
# End Source File
# Begin Source File

SOURCE=.\UAS.rc
# End Source File
# Begin Source File

SOURCE=.\resource\UAS2.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\blobage.txt
# End Source File
# Begin Source File

SOURCE=.\UAS2_ver.dsm
# End Source File
# End Target
# End Project
