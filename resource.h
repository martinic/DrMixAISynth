// Double quotes, spaces OK.
#define PLUG_MFR  "Martinic"
#define PLUG_NAME "Doctor Mix AI Synth"

// No quotes or spaces.
#define PLUG_CLASS_NAME DrMixAISynth

// To-do: Set AudioComponents description and name in AU-Info.plist to
// PLUG_NAME " AU" and PLUG_MFR ": " PLUG_NAME.

// Double quotes, no spaces. BUNDLE_DOMAIN must contain only alphanumeric
// (A-Z, a-z, 0-9), hyphen (-), and period (.) chars. BUNDLE_NAME must be
// less than 16 chars.
#define BUNDLE_DOMAIN "com.Martinic"
#define BUNDLE_NAME   "DrMixAISynth"

#ifdef __APPLE__
	// To-do: Set Product Bundle Indentifier in Xcode project to
	// BUNDLE_DOMAIN.audiounit.BUNDLE_NAME (AU target),
	// BUNDLE_DOMAIN.clap.BUNDLE_NAME (CLAP target), and
	// BUNDLE_DOMAIN.vst.BUNDLE_NAME (VST2 target).

	// No spaces.
	#define PLUG_FACTORY    DrMixAISynth_Factory
	#define PLUG_ENTRY      DrMixAISynth_Entry
	#define PLUG_ENTRY_STR "DrMixAISynth_Entry"
	#define VIEW_CLASS      DrMixAISynth_View
	#define VIEW_CLASS_STR "DrMixAISynth_View"

	// To-do: Create export file with two entries: PLUG_ENTRY and
	// PLUG_FACTORY, each with leading underscore (e.g. DrMixAISynth.exp
	// with entries _DrMixAISynth_Entry and _DrMixAISynth_Factory).

	// To-do: Set NSPrincipalClass in AU-Info.plist to VIEW_CLASS.
#endif

// Interpreted as 0xMajrMnBg.
#define PLUG_VER  0x00010000 // 1.0.0
#define VERSIONINFO_VERSION     1,0,0,0
#define VERSIONINFO_STR        "1.0.0 RC1"

#define VERSIONINFO_PRERELEASE // Define if alpha, beta, or RC
#define VERSIONINFO_COPYRIGHT "2023 Martinic"

// Windows version info resource (optional).
#ifdef _WIN32
	#define VERSIONINFO_ORG_FILE PLUG_NAME

// Apple version info (non-optional).
#elif defined(__APPLE__)
	// To-do: Set INFOPLIST_KEY_NSHumanReadableCopyright and
	// MARKETING_VERSION in Xcode project to VERSIONINFO_COPYRIGHT and
	// VERSIONINFO_STR.

	// To-do: Set AudioComponents version in AU-Info.plist to PLUG_VER.
#endif

// 4 chars, single quotes.
#define PLUG_UNIQUE_ID 'dra1'
#define PLUG_MFR_ID    'Mnic'

// Note: VST2 IDs can no longer be registered, so choose wisely to avoid
// possible clashes with other VST2 plugins.

// To-do: Set CFBundleSignature in AU/VST2-Info.plist to PLUG_UNIQUE_ID.

// To-do: Set AudioComponents manufacturer and subtype in AU-Info.plist to
// PLUG_MFR_ID and PLUG_UNIQUE_ID.

// Plugin I/O config.
#define PLUG_CHANNEL_IO "0-2"

// Latency in samples.
#define PLUG_LATENCY 0

// Plugin flags.
#define PLUG_IS_INST       1
#define PLUG_DOES_MIDI_IN  1
#define PLUG_DOES_MIDI_OUT 0

// To-do: Set AudioComponents type in AU-Info.plist to aufx (audio effect),
// aumu (instrument), or aumf (audio effect with MIDI input).

// Unique IDs for each image resource, bit 0 is scale (0 = full, 1 = half).

#define BACKGROUND_2X_PNG_ID 100
#define SWITCH_2X_PNG_ID     102
#define KNOB_2X_PNG_ID       104

#define BACKGROUND_PNG_ID    101
#define SWITCH_PNG_ID        103
#define KNOB_PNG_ID          105

// Image resource filenames (case-sensitive!).

#define BACKGROUND_2X_PNG_FN "img/Background@2x.png"
#define SWITCH_2X_PNG_FN     "img/Switch@2x.png"
#define KNOB_2X_PNG_FN       "img/Knob@2x.png"

#define BACKGROUND_PNG_FN    "img/Background.png"
#define SWITCH_PNG_FN        "img/Switch.png"
#define KNOB_PNG_FN          "img/Knob.png"

// To-do: Set AudioComponents type in AU-Info.plist to aufx (audio effect),
// aumu (instrument), or aumf (audio effect with MIDI input).
