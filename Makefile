# Usage: nmake [config=Release|Debug] [ndebug=1] ...

PROJECT = DrMixAISynth
OUTFILE = Doctor Mix AI Synth

CFLAGS = /fp:fast /D _USE_MATH_DEFINES /I . /c /Fd"$(OUTDIR)/" /Zi /W3 /WX /D _CRT_SECURE_NO_WARNINGS /D _CRT_NONSTDC_NO_WARNINGS /MP /nologo
CPPFLAGS = /EHsc $(CFLAGS)

RCFLAGS = /nologo
LINKFLAGS = /dll /subsystem:windows /dynamicbase:no /manifest:no /nologo

!IF "$(TARGET_CPU)" == "x64" || "$(CPU)" == "AMD64" || "$(PLATFORM)" == "x64" || "$(PLATFORM)" == "X64"

PLATFORM = x64
CFLAGS = $(CFLAGS) /favor:blend
LINKFLAGS = $(LINKFLAGS) /machine:x64

!ELSE

PLATFORM = Win32
CFLAGS = $(CFLAGS) /arch:SSE2
LINKFLAGS = $(LINKFLAGS) /machine:x86

!ENDIF

!IF DEFINED(CONFIG)
CONFIGURATION = $(CONFIG)
!ENDIF

!IF !DEFINED(CONFIGURATION) && DEFINED(CPU) && DEFINED(APPVER) && !DEFINED(NODEBUG)
CONFIGURATION = Debug
!ELSE IF !DEFINED(CONFIGURATION)
CONFIGURATION = Release
!ENDIF

!IF "$(CONFIGURATION)" == "Release"

!IF DEFINED(NDEBUG) && "$(NDEBUG)" != "0"
CFLAGS = $(CFLAGS) /D NDEBUG /GL
LINKFLAGS = $(LINKFLAGS) /ltcg
!ELSE
LINKFLAGS = $(LINKFLAGS) /debug
!ENDIF

CFLAGS = $(CFLAGS) /O2 /Gy /GS- /GR- /MT
LINKFLAGS = $(LINKFLAGS) /incremental:no /opt:ref /opt:icf /defaultlib:libcmt

!ELSE IF "$(CONFIGURATION)" == "Debug"

CFLAGS = $(CFLAGS) /D _DEBUG /D DEBUG /RTCsu /MTd
LINKFLAGS = $(LINKFLAGS) /debug

!ENDIF

OUTDIR = $(PLATFORM)/$(CONFIGURATION)

!MESSAGE $(PROJECT) - $(CONFIGURATION)|$(PLATFORM)
!MESSAGE

all : clap

"$(OUTDIR)" :
!IF !EXIST("$(OUTDIR)/")
	mkdir $@
!ENDIF

IPLUGINC = \
IPlug/Containers.h \
IPlug/Hosts.h \
IPlug/IControl.h \
IPlug/IGraphics.h \
IPlug/IGraphicsWin.h \
IPlug/IParam.h \
IPlug/IPlug_include_in_plug_hdr.h \
IPlug/IPlug_include_in_plug_src.h \
IPlug/IPlugBase.h \
IPlug/IPlugStructs.h

SOURCES = \
"$(PROJECT).cpp" \
"$(PROJECT).h" \
resource.h \
$(IPLUGINC)

"$(OUTDIR)/$(PROJECT)_CLAP.obj" : $(SOURCES) IPlug/IPlugCLAP.h
	$(CPP) $(CPPFLAGS) /D CLAP_API /wd4244 /Fo$@ /Fa"$(OUTDIR)/_$(PROJECT)_CLAP.asm" "$(PROJECT).cpp"

"$(OUTDIR)/$(PROJECT)_VST2.obj" : $(SOURCES) IPlug/IPlugVST2.h
	$(CPP) $(CPPFLAGS) /D VST2_API /wd4244 /Fo$@ /Fa"$(OUTDIR)/_$(PROJECT)_VST2.asm" "$(PROJECT).cpp"

RESOURCES = \
"$(PROJECT).rc" \
resource.h \
\
img/Background.png \
img/Switch.png \
img/Knob.png \
\
img/Background@2x.png \
img/Switch@2x.png \
img/Knob@2x.png

"$(OUTDIR)/$(PROJECT)_CLAP.res" : $(RESOURCES)
	$(RC) $(RCFLAGS) /D CLAP_API /fo$@ "$(PROJECT).rc"

"$(OUTDIR)/$(PROJECT)_VST2.res" : $(RESOURCES)
	$(RC) $(RCFLAGS) /D VST2_API /fo$@ "$(PROJECT).rc"

IPLUGOBJ = \
"$(OUTDIR)/Hosts.obj" \
"$(OUTDIR)/IControl.obj" \
"$(OUTDIR)/IGraphics.obj" \
"$(OUTDIR)/IGraphicsWin.obj" \
"$(OUTDIR)/IParam.obj" \
"$(OUTDIR)/IPlugBase.obj" \
"$(OUTDIR)/IPlugStructs.obj"

iplug : "$(OUTDIR)" $(IPLUGOBJ) "$(OUTDIR)/IPlugCLAP.obj" "$(OUTDIR)/IPlugVST2.obj"

{IPlug}.cpp{$(OUTDIR)}.obj ::
	$(CPP) $(CPPFLAGS) /Fo"$(OUTDIR)/" $<

"$(OUTDIR)/IPlugVST2.obj" : IPlug/IPlugVST2.cpp
	$(CPP) $(CPPFLAGS) /D IPLUG_LEGACY_PLUG_VER /Fo"$(OUTDIR)/" $**

LIBPNGOBJ = \
"$(OUTDIR)/png.obj" \
"$(OUTDIR)/pngerror.obj" \
"$(OUTDIR)/pngget.obj" \
"$(OUTDIR)/pngmem.obj" \
"$(OUTDIR)/pngread.obj" \
"$(OUTDIR)/pngrio.obj" \
"$(OUTDIR)/pngrtran.obj" \
"$(OUTDIR)/pngrutil.obj" \
"$(OUTDIR)/pngset.obj" \
"$(OUTDIR)/pngtrans.obj"

libpng : "$(OUTDIR)" $(LIBPNGOBJ)

{WDL/libpng}.c{$(OUTDIR)}.obj ::
	$(CC) $(CFLAGS) /Fo"$(OUTDIR)/" $<

LICEOBJ = \
"$(OUTDIR)/lice.obj" \
"$(OUTDIR)/lice_arc.obj" \
"$(OUTDIR)/lice_line.obj" \
"$(OUTDIR)/lice_png.obj" \
"$(OUTDIR)/lice_textnew.obj"

lice : "$(OUTDIR)" $(LICEOBJ)

{WDL/lice}.cpp{$(OUTDIR)}.obj ::
	$(CPP) $(CPPFLAGS) /Fo"$(OUTDIR)/" $<

ZLIBOBJ = \
"$(OUTDIR)/adler32.obj" \
"$(OUTDIR)/crc32.obj" \
"$(OUTDIR)/infback.obj" \
"$(OUTDIR)/inffast.obj" \
"$(OUTDIR)/inflate.obj" \
"$(OUTDIR)/inftrees.obj" \
"$(OUTDIR)/trees.obj" \
"$(OUTDIR)/zutil.obj"

zlib : "$(OUTDIR)" $(ZLIBOBJ)

{WDL/zlib}.c{$(OUTDIR)}.obj ::
	$(CC) $(CFLAGS) /D NO_GZCOMPRESS /D Z_SOLO /Fo"$(OUTDIR)/" $<

OBJECTS = \
$(IPLUGOBJ) \
$(LIBPNGOBJ) \
$(LICEOBJ) \
$(ZLIBOBJ)

LIBS = \
advapi32.lib \
comctl32.lib \
comdlg32.lib \
gdi32.lib \
ole32.lib \
shell32.lib \
user32.lib \
wininet.lib

"$(OUTDIR)/$(OUTFILE).clap" : "$(OUTDIR)/$(PROJECT)_CLAP.obj" "$(OUTDIR)/$(PROJECT)_CLAP.res" "$(OUTDIR)/IPlugCLAP.obj" $(OBJECTS)
	@echo ^ ^ ^ ^ ^ ^ ^ ^ link $(LINKFLAGS) /out:$@ "$(OUTDIR)/$(PROJECT)_CLAP.obj" ...
	@link $(LINKFLAGS) /out:$@ /implib:"$(OUTDIR)/$(PROJECT)_CLAP.lib" $** $(LIBS)

"$(OUTDIR)/$(OUTFILE).dll" : "$(OUTDIR)/$(PROJECT)_VST2.obj" "$(OUTDIR)/$(PROJECT)_VST2.res" "$(OUTDIR)/IPlugVST2.obj" $(OBJECTS)
	@echo ^ ^ ^ ^ ^ ^ ^ ^ link $(LINKFLAGS) /out:$@ "$(OUTDIR)/$(PROJECT)_VST2.obj" ...
	@link $(LINKFLAGS) /out:$@ /implib:"$(OUTDIR)/$(PROJECT)_VST2.lib" $** $(LIBS)

clap : "$(OUTDIR)" "$(OUTDIR)/$(OUTFILE).clap"

vst2 : "$(OUTDIR)" "$(OUTDIR)/$(OUTFILE).dll"

dist : clap vst2
!IFDEF REMINDER
	@echo.
!	IF "$(PLATFORM)" == "x64"
	@echo $(REMINDER:x64_or_x86=x86)
!	ELSE
	@echo $(REMINDER:x64_or_x86=x64)
!	ENDIF
!ENDIF

clean :
!IF EXIST("$(OUTDIR)/")
	rmdir /s /q "$(OUTDIR)"
!ENDIF
!IF EXIST("$(PLATFORM)/")
	-@rmdir /q "$(PLATFORM)"
!ENDIF
