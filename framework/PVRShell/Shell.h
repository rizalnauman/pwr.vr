/*!
\brief Contains the all-important pvr::Shell class that the user will be inheriting from for his application. See
bottom of this file or of any Demo file for the newDemo function the user must implement for his application.
\file PVRShell/Shell.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRShell/CommandLine.h"
#include "PVRCore/Base/NativeLibrary.h"

#include <bitset>


/// <summary>Main namespace for the PowerVR Framework</summary>
namespace pvr {

//!\cond NO_DOXYGEN
struct PointerLocationStore
{
	int16 x; int16 y;
	PointerLocationStore operator +(const PointerLocationStore& rhs)
	{
		return PointerLocationStore{ static_cast<int16>(x + rhs.x), static_cast<int16>(y + rhs.y)};
	}

	PointerLocationStore operator -(const PointerLocationStore& rhs)
	{
		return PointerLocationStore{ static_cast<int16>(x - rhs.x), static_cast<int16>(y - rhs.y)};
	}

	void operator +=(const PointerLocationStore& rhs)
	{
		*this  = *this + rhs;
	}

	void operator -=(const PointerLocationStore& rhs)
	{
		*this  = *this - rhs;
	}

};
//!\endcond

/// <summary>Mouse pointer coordinates.</summary>
class PointerLocation : public PointerLocationStore
{
public:
	PointerLocation() { }
	PointerLocation(const PointerLocationStore& st) : PointerLocationStore(st) { }
	PointerLocation(int16 x, int16 y) { this->x = x; this->y = y; }
};

/// <summary>Enumeration representing a simplified, unified input event designed to unify simple actions across different
/// devices.</summary>
enum class SimplifiedInput
{
	NONE = 0,  //!<No action - avoid using
	Left = 1,  //!<Left arrow, Swipe left
	Right = 2, //!<Right arrow, Swipe left
	Up = 3,   //!<Up arrow, Swipe left
	Down = 4, //!<Down arrow, Swipe left
	ActionClose = 5, //!<Esc, Q, Android back, iOS home
	Action1 = 6,   //!<Space, Enter, Touch screen center
	Action2 = 7,  //!<Key 1, Touch screen left side
	Action3 = 8,  //!<Key 2, Touch screen right side
};

/// <summary>Enumeration representing a System Event (quit, Gain focus, Lose focus).</summary>
enum class SystemEvent
{
	SystemEvent_Quit, SystemEvent_LoseFocus, SystemEvent_GainFocus
};

/// <summary>Enumeration representing a Keyboard Key.</summary>
enum class Keys : byte
{
//Whenever possible, keys get ASCII values of their default (non-shifted) values of a default US keyboard.
	Backspace = 0x08,
	Tab = 0x09,
	Return = 0x0D,

	Shift = 0x10, Control = 0x11, Alt = 0x12,

	Pause = 0x13,
	PrintScreen = 0x2C,
	CapsLock = 0x14,
	Escape = 0x1B,
	Space = 0x20,

	PageUp = 0x21, PageDown = 0x22, End = 0x23, Home = 0x24,

	Left = 0x25, Up = 0x26, Right = 0x27, Down = 0x28,

	Insert = 0x2D, Delete = 0x2E,

	//ASCII-Based
	Key0 = 0x30, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8, Key9,

	A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M,
	N = 0x4E, O, P, Q, R, S, T, U, V, W, X, Y, Z,


	Num0 = 0x60, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, NumPeriod,

	F1 = 0x70, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

	SystemKey1 = 0x5B, SystemKey2 = 0x5D,
	WindowsKey = 0x5B, MenuKey = 0x5D, //ALIASES

	NumMul = 0x6A, NumAdd = 0x6B, NumSub = 0x6D, NumDiv = 0x6E,
	NumLock = 0x90, ScrollLock = 0x91,

	Semicolon = 0xBA, Equals = 0xBB, Minus = 0xBD,

	Slash = 0xBF,

	Comma = 0xBC, Period = 0xBE,

	Backquote = 0xC0,

	SquareBracketLeft = 0xDB, SquareBracketRight = 0xDD, Quote = 0xDE, Backslash = 0xDC,

	MaxNumberOfKeyCodes,
	Unknown = 0xFF
};


struct ConfigureEvent
{
	int x, y;
	int width, height;
	int border_width;
};

class Stream;
/// <summary>Contains system/platform related classes and namespaces. Main namespace for the PowerVR Shell.</summary>
namespace platform {

/// <summary>Contains all data of a system event.</summary>
struct ShellEvent
{
	enum
	{
		SystemEvent,
		PointingDeviceDown,
		PointingDeviceUp,
		PointingDeviceMove,
		KeyDown,
		KeyUp
	} type;

	union
	{
		PointerLocationStore location;
		uint8 buttonIdx;
		pvr::SystemEvent systemEvent;
		Keys key;
	};
};

struct ShellData;
class ShellOS;
/// <summary>The PowerVR Shell (pvr::Shell) is the main class that the user will inherit his application from.
/// </summary>
/// <remarks>This class abstracts the platform for the user and provides a unified interface to it. The user will
/// normally write his application as a class inheriting from the Shell. This way the user can have specific and
/// easy to use places to write his code - Application start, window initialisation, per frame, cleanup. All
/// platform queries and settings can be done on the shell (set the required Graphics API required, window size
/// etc.). Specific callbacks and queries are provided for system events (keyboard, mouse, touch) as well as a
/// unified simplified input interface provided such abstracted input events as "Left", "Right", "Action1", "Quit"
/// across different platforms.</remarks>
class Shell : public IPlatformProvider
{
	friend class ShellOS;
	friend class StateMachine;
public:
	/// <summary>Contains a pointer location in normalised coordinates.</summary>
	struct PointerNormalisedLocation
	{
		PointerNormalisedLocation() {}
		PointerNormalisedLocation(const PointerLocation& location) { x = location.x; y = location.y; }

		float32 x; float32 y;
	};

	/// <summary>Contains the state of a pointing device (mouse, touch screen).</summary>
	struct PointingDeviceState
	{
	protected:
		PointerLocation pointerLocation;    //!< Location of the pointer
		PointerLocation dragStartLocation;  //!< Location of a drag starting point
		int8 buttons;                       //!< Buttons pressed
	public:
		/// <summary>Constructor.</summary>
		PointingDeviceState() : pointerLocation(0, 0), dragStartLocation(0, 0), buttons(0) {}
		/// <summary>Get the current (i.e. last known) location of the mouse/pointing device pointer.</summary>
		/// <returns>The location of the pointer.</returns>
		PointerLocation position() { return pointerLocation; }

		/// <summary>Get the location of the mouse/pointing device pointer when the last drag started.</summary>
		/// <returns>The location of a drag action's starting point.</returns>
		PointerLocation dragStartPosition() { return dragStartLocation; }

		/// <summary>Query if a specific button is pressed.</summary>
		/// <param name="buttonIndex">The index of the button (0 up to 6).</param>
		/// <returns>True if the button exists and is pressed. False otherwise.</returns>
		bool isPressed(int8 buttonIndex)
		{
			return (buttons & (1 << buttonIndex)) != 0;
		}
		/// <summary>Check if a drag action has started.</summary>
		/// <returns>True if during a dragging action.</returns>
		bool isDragging()
		{
			return (buttons & 0x80) != 0;
		}

	};

	/// <summary>Internal class of the pvr::Shell.</summary>
	struct PrivatePointerState : public PointingDeviceState
	{
		void startDragging() { buttons |= 0x80; dragStartLocation = pointerLocation; }
		void endDragging() { buttons &= 0x7F; }

		void setButton(int8 buttonIndex, bool pressed)
		{
			buttons = (buttons & ~(1 << buttonIndex)) | (pressed << buttonIndex);
		}
		void setPointerLocation(PointerLocation pointerLocation)
		{
			this->pointerLocation = pointerLocation;
		}

	};

protected:

	/// <summary>IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event represents application start.</summary>
	/// <returns>When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned , the Shell will detect that, clean up, and exit.</returns>
	/// <remarks>This function must be implemented in the user's application class. It will be fired once, on start,
	/// before any other callback and before Graphics Context aquisition. It is suitable to do per-run initialisation,
	/// load assets files and similar tasks. A context does not exist yet, hence if the user tries to create API
	/// objects, they will fail and the behaviour is undefined.</remarks>
	virtual Result initApplication() = 0;

	/// <summary>IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event is called after successful window/context
	/// aquisition.</summary>
	/// <returns>When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned , the Shell will detect that, clean up, and exit.</returns>
	/// <remarks>This function must be implemented in the user's application class. It will be fired once after every
	/// time the main Graphics Context (the one the Application Window is using) is initialized. This is usually once
	/// per application run, but in some cases (context lost) it may be called more than once. If the context is lost,
	/// the releaseView() callback will be fired, and if it is reaquired this function will be called again. This
	/// callback is suitable to do all do-once tasks that require a graphics context, such as creating an On-Screen
	/// FBO, and for simple applications creating the graphics objects.</remarks>
	virtual Result initView() = 0;

	/// <summary>IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event is called every frame.</summary>
	/// <returns>When implementing, return a suitable error code to signify failure. Return pvr::Success to signify
	/// success and allow the Shell to do all actions necessary to render the frame (swap buffers etc.). If
	/// pvr::Result::Success is not returned, the Shell will detect that, clean up, and exit. Return
	/// pvr::Result::ExitRenderFrame to signify a clean, non-error exit for the application. Any other error code will
	/// be logged.</returns>
	/// <remarks>This function must be implemented in the user's application class. It will be fired once every frame.
	/// The user should use this callback as his main callback to start rendering and per-frame code. This callback is
	/// suitable to do all per-frame task. In multithreaded environments, it should be used to mark the start and
	/// signal the end of frames.</remarks>
	virtual Result renderFrame() = 0;

	/// <summary>IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event represents graphics context released.
	/// </summary>
	/// <returns>When implementing, return a suitable error code to signify failure. If pvr::Result::Success is not
	/// returned, the Shell will detect that, clean up, and exit. If the shell was exiting, this will happen anyway.
	/// </returns>
	/// <remarks>This function must be implemented in the user's application class. It will be fired once before the
	/// main Graphics Context is lost. The user should use this callback as his main callback to release all API
	/// objects as they will be invalid afterwards. In simple applications where all objects are created in initView,
	/// it should release all objects acquired in initView. This callback will be called when the application is
	/// exiting, but not only then - losing (and later re-acquiring) the Graphics Context will lead to this callback
	/// being fired, followed by an initView callback, renderFrame etc.</remarks>
	virtual Result releaseView() = 0;

	/// <summary>IMPLEMENT THIS FUNCTION IN YOUR APPLICATION CLASS. This event represents application exit.</summary>
	/// <returns>When implementing, return a suitable error code to signify a failure that will be logged.</returns>
	/// <remarks>This function must be implemented in the user's application class. It will be fired once before the
	/// application exits, after the Graphics Context is torn down. The user should use this callback as his main
	/// callback to release all objects that need to. The application will exit shortly after this callback is fired.
	/// In effect, the user should release all objects that were acquired during initApplication. Do NOT use this to
	/// release API objects - these should already have been released during releaseView.</remarks>
	virtual Result quitApplication() = 0;

	/// <summary>Override in your class to handle the "Click" or "Touch" event of the main input device (mouse or
	/// touchscreen).</summary>
	/// <param name="buttonIdx">The index of the button (LMB:0, RMB:1, MMB:2, Touch:0)</param>
	/// <param name="location">The location of the click.</param>
	/// <remarks>This event will be fired on releasing the button, when the mouse pointer has not moved more than a few
	/// pixels since the button was pressed (otherwise a drag will register instead of a click).</remarks>
	virtual void eventClick(int buttonIdx, PointerLocation location) { (void)buttonIdx; (void)location; }

	/// <summary>Override in your class to handle the finish of a "Drag" event of the main input device (mouse,
	/// touchscreen).</summary>
	/// <param name="location">The location of the click.</param>
	/// <remarks>This event will be fired on releasing the button, and the mouse pointer has not moved more than a few
	/// pixels since the button was pressed.</remarks>
	virtual void eventDragFinished(PointerLocation location) { (void)location; }

	/// <summary>Override in your class to handle the start of a "Drag" event of the main input device (mouse,
	/// touchscreen).</summary>
	/// <param name="buttonIdx">The index of the button (LMB:0, RMB:1, MMB:2, Touch:0).</param>
	/// <param name="location">The location of the click.</param>
	/// <remarks>This event will be fired after a movement of more than a few pixels is detected with a button down.
	/// </remarks>
	virtual void eventDragStart(int buttonIdx, PointerLocation location) { (void)location; (void)buttonIdx; }

	/// <summary>Override in your class to handle the initial press (down) of the main input device (mouse, touchscreen).
	/// </summary>
	/// <param name="buttonIdx">The index of the button (LMB:0, RMB:1, MMB:2, Touch:0)</param>
	/// <remarks>This event will be fired on pressing any button.</remarks>
	virtual void eventButtonDown(int buttonIdx) { (void)buttonIdx; }

	/// <summary>Override in your class to handle the release (up) of the main input device (mouse, touchscreen).
	/// </summary>
	/// <param name="buttonIdx">The index of the button (LMB:0, RMB:1, MMB:2, Touch:0)</param>
	/// <remarks>This event will be fired on releasing any button.</remarks>
	virtual void eventButtonUp(int buttonIdx) { (void)buttonIdx; }

	/// <summary>Override in your class to handle the press of a key of the keyboard.</summary>
	/// <param name="key">The key pressed</param>
	/// <remarks>This event will be fired on pressing any key.</remarks>
	virtual void eventKeyDown(Keys key) { (void)key; }

	/// <summary>Override in your class to handle a keystroke of the keyboard.</summary>
	/// <param name="key">The key pressed</param>
	/// <remarks>This event will normally be fired multiple times during a key press, as controlled by the key repeat
	/// of the operating system.</remarks>
	virtual void eventKeyStroke(Keys key) { (void)key; }

	/// <summary>Override in your class to handle the release (up) of a key of of the keyboard.</summary>
	/// <param name="key">The key released</param>
	/// <remarks>This event will be fired once, when releasing a key.</remarks>
	virtual void eventKeyUp(Keys key) { (void)key; }

	/// <summary>Override in your class to handle a unified interface for input across different platforms and devices.
	/// </summary>
	/// <param name="key">The Simplified Unified Event</param>
	/// <remarks>This event abstracts, maps and unifies several input devices, in a way with a mind to unify several
	/// platforms and input devices. The Left/Right/Up/Down keyboard key, Swipe Left/Right/Up/Down both cause
	/// Left/Right/Up/Down events. Left Click at Center, Space key, Enter key, Touch at Center cause Action1. Left
	/// Click at Left, Right Click, One Key, Touch at the Left cause Action2. Left Click at Right, Middle Click, Two
	/// Key, Touch at the Right cause Action3. Escape, Q key, Back button cause Quit. Default behaviour is Quit action
	/// calls exitShell. In order to retain Quit button functionality, this behaviour should be mirrored (exitShell
	/// called on ActionClose).</remarks>
	virtual void eventMappedInput(SimplifiedInput key)
	{
		switch (key)
		{
		case SimplifiedInput::ActionClose:
			exitShell();
			break;
		default: break;
		}
	}

public:
	/// <summary>Used externally to signify events to the Shell. Do not use.</summary>
	void onKeyDown(Keys key)
	{
		ShellEvent evt;
		evt.type = ShellEvent::KeyDown;
		evt.key = key;
		eventQueue.push(evt);
	}

	/// <summary>Used externally to signify events to the Shell. Do not use.</summary>
	void onKeyUp(Keys key)
	{
		ShellEvent evt;
		evt.type = ShellEvent::KeyUp;
		evt.key = key;
		eventQueue.push(evt);
	}

	/// <summary>Used externally to signify events to the Shell. Do not use.</summary>
	void onPointingDeviceDown(uint8 buttonIdx)
	{
		ShellEvent evt;
		evt.type = ShellEvent::PointingDeviceDown;
		evt.buttonIdx = buttonIdx;
		eventQueue.push(evt);
	}

	/// <summary>Used externally to signify events to the Shell. Do not use.</summary>
	void onPointingDeviceUp(uint8 buttonIdx)
	{
		ShellEvent evt;
		evt.type = ShellEvent::PointingDeviceUp;
		evt.buttonIdx = buttonIdx;
		eventQueue.push(evt);
	}

	/// <summary>Used externally to signify events to the Shell. Do not use.</summary>
	void onSystemEvent(SystemEvent systemEvent)
	{
		ShellEvent evt;
		evt.type = ShellEvent::SystemEvent;
		evt.systemEvent = systemEvent;
		eventQueue.push(evt);
	}

	void onConfigureEvent(const ConfigureEvent& configureEvent)
	{
		_configureEvent = configureEvent;
	}


private:
	/// <summary>Used externally to signify events to the Shell. Do not use.</summary>
	void updatePointerPosition(PointerLocation location);
	void implKeyDown(Keys key);
	void implKeyUp(Keys key);
	void implPointingDeviceDown(uint8 buttonIdx);
	void implPointingDeviceUp(uint8 buttonIdx);
	void implSystemEvent(SystemEvent systemEvent);

	std::queue<ShellEvent> eventQueue;
	std::auto_ptr<native::NativeLibrary> pvrapi;

	void processShellEvents();

public:
	/// <summary>Default constructor. Do not instantiate a Shell class directly - extend as your application and then
	/// provide the newDemo() function returning your application instance. See bottom of this file.</summary>
	Shell();

	/// <summary>Called at the appropriate time by the state machine.</summary>
	Result init(ShellData* data);

	/// <summary>Destructor.</summary>
	virtual ~Shell();

	/// <summary>Get the display attributes (width, height, bpp, AA, etc) of this pvr::Shell</summary>
	/// <returns>The display attributes (width, height, bpp, AA, etc) of this pvr::Shell</returns>
	/// <remarks>OSManager interface implementation.</remarks>
	DisplayAttributes& getDisplayAttributes();

	/// <summary>Get the underlying Display object of this shell</summary>
	/// <returns>The underlying Display object of this shell</returns>
	/// <remarks>OSManager interface implementation.</remarks>
	OSDisplay getDisplay();

	/// <summary>Get the underlying Window object of this shell</summary>
	/// <returns>The underlying Window object of this shell</returns>
	/// <remarks>OSManager interface implementation.</remarks>
	OSWindow getWindow();
	/* END IMPLEMENT OSManager*/


private:
	/* called by our friend the State Machine */
	Result shellInitApplication();
	Result shellQuitApplication();
	Result shellInitView();
	Result shellReleaseView();
	Result shellRenderFrame();

public:
	/// <summary>Query if a key is pressed.</summary>
	/// <param name="key">The key to check</param>
	/// <returns>True if a keyboard exists and the key is pressed</returns>
	bool isKeyPressed(Keys key) { return isKeyPressedVal(key); }

	/// <summary>Query if a key is pressed.</summary>
	/// <param name="buttonIndex">The number of the button to check (LMB:0, RMB:1, MMB:2)</param>
	/// <returns>True if a mouse/touchscreen exists and the button with this is index pressed. Simple touch is 0.
	/// </returns>
	bool isButtonPressed(int8 buttonIndex) { return buttonIndex > 7 ? false : _pointerState.isPressed(buttonIndex); }

	/// <summary>Query the pointer location in pixels.</summary>
	/// <returns>The location of the pointer in pixels.</returns>
	PointerLocation getPointerAbsolutePosition() { return _pointerState.position(); }

	PointerLocation getPointerRelativePosition()
	{
		PointerLocation pointerloc = getPointerAbsolutePosition();
		pointerloc.x -= _configureEvent.x;
		pointerloc.y -= _configureEvent.y;
		return pointerloc;
	}


	/// <summary>Query the pointer location in normalised coordinates (0..1).</summary>
	/// <returns>The location of the pointer in normalised coordinates (0..1).</returns>
	PointerNormalisedLocation getPointerNormalisedPosition()
	{
		PointerNormalisedLocation pos;
		pos.x = _pointerState.position().x / (float)getWidth();
		pos.y = _pointerState.position().y / (float)getHeight();
		return pos;
	}

	/// <summary>Query the state of the pointing device (Mouse, Touchscreend).</summary>
	/// <returns>A PointingDeviceState struct containing the state of the pointing device.</returns>
	PointingDeviceState& getPointingDeviceState() { return _pointerState; }
	/* End Input Handling :  Queried */

	/// <summary>Get the total time (from the same arbitrary starting point as getTimeAtInitApplication ), in
	/// milliseconds.</summary>
	/// <returns>The total time in milliseconds.</returns>
	uint64 getTime();

	/// <summary>The duration of the last frame to pass, in milliseconds. This is the time to use to advance app
	/// logic.</summary>
	/// <returns>The duration of the last frame, in milliseconds.</returns>
	uint64 getFrameTime();

	/// <summary>Get the total time (from the same arbitrary starting point as getTime ), in milliseconds.</summary>
	/// <returns>The time at init application, in milliseconds.</returns>
	uint64 getTimeAtInitApplication() const;

	/// <summary>Get the command line options that were passed at application launch.</summary>
	/// <returns>A pvr::platform::CommandLineParser::ParsedCommandLine object containing all options passed at app
	/// launch.</returns>
	const platform::CommandLineParser::ParsedCommandLine& getCommandLine() const;

	/// <summary>ONLY EFFECTIVE AT INIT APPLICATION. Set the application to run at full screen mode. Not all platforms
	/// support this option.</summary>
	/// <param name="fullscreen">Set to true for fullscreen, false for windowed</param>
	void setFullscreen(const bool fullscreen);
	/// <summary>Get if the application is running in full screen.</summary>
	/// <returns>True if the application is running in full screen. False otherwise.</returns>
	bool isFullScreen() const;

	/// <summary>Get the width of the application area (window width for windowed, or screen width for full screen).
	/// </summary>
	/// <returns>The width of the application area.</returns>
	uint32 getWidth() const;

	/// <summary>Get the height of the application area (window height for windowed, or screen height for full
	/// screen).</summary>
	/// <returns>The height of the application area.</returns>
	uint32 getHeight() const;

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the window size, or resolution for fullscreen.
	/// </summary>
	/// <param name="w">The width of the window / horizontal resolution</param>
	/// <param name="h">The height of the window / vertical resolution</param>
	/// <returns>pvr::Result::Success if successful. pvr::Result::UnsupportedRequest if unsuccessful.</returns>
	Result setDimensions(uint32 w, uint32 h);

	/// <summary>Get the window position X coordinate.</summary>
	/// <returns>The window position X coordinate. (0 for fullscreen)</returns>
	uint32 getPositionX() const;

	/// <summary>Get the window position Y coordinate.</summary>
	/// <returns>The window position Y coordinate. (0 for fullscreen)</returns>
	uint32 getPositionY() const;

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the window position. Not supported in all platforms.
	/// </summary>
	/// <returns>pvr::Result::Success if successful. pvr::Result::UnsupportedRequest if unsuccessful.</returns>
	Result setPosition(uint32 x, uint32 y);

	/// <summary>Return the frame after which the application is set to automatically quit. If QuitAfterFrame was not
	/// set, returns -1</summary>
	/// <returns>The frame after which the application is set to quit. If not set, returns -1</returns>
	int32 getQuitAfterFrame() const;
	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set a frame after which the application will quit.
	/// </summary>
	/// <param name="value">The frame after which the application is set to quit. Set to -1 to disable.</param>
	void setQuitAfterFrame(uint32 value);

	/// <summary>Get the time after which the application is set to automatically quit.If QuitAfterTime was not set,
	/// returns -1</summary>
	/// <returns>The time after which the application will quit. If not set, returns -1</returns>
	float32 getQuitAfterTime() const;
	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set a time after which the application will quit.
	/// </summary>
	/// <param name="value">The time (seconds) after which the application will quit. Set to -1 to disable.</param>
	void setQuitAfterTime(float32 value);

	/// <summary>Get the vertical synchronization mode.</summary>
	/// <returns>The vertical synchronisation mode.</returns>
	VsyncMode getVsyncMode() const;

	/// <summary>Get the number of explicit, logical framebuffer images accessible to user code. OpenGL ES will always
	/// return 1 as the buffer swapping is hidden in the driver and images not explicitly handled by the user.
	/// </summary>
	/// <returns>The number of logical swap chain images. This number is one greater than the max number returned by
	/// getSwapChainIndex().</returns>
	uint32 getSwapChainLength() const;

	/// <summary>Get the logical framebuffer image that the application currently owns and can render to. OpenGL ES
	/// will always return 0 as the backbuffer images imlementation are hidden in the driver and not exposed to the
	/// user.</summary>
	/// <returns>The index of the logical backbuffer image that the application currently owns and should render to.
	/// It is undefined behaviour to render to an Off Screen FBO that points to any swap image other than the one
	/// whose index is this number.</returns>
	uint32 getSwapChainIndex() const;

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the Vertical Syncchronization mode(vertical sync).
	/// Default is On(Fifo).</summary>
	/// <param name="mode">The Vertical Synchronization mode. <para>On : VerticalSync (no tearing, some possible input
	/// lag</para> <para>Off : no synchronization. Little lag, bad tearing.</para> <para>Mailbox: If supported, what
	/// was commonly known as triple-buffering - the app uses the latest full image. Little lag, no tearing.</para>
	/// <para>Relaxed: If supported, the presentation engine will only synchronize if framerate is greater than
	/// refresh. Some lag, minimal tearing.</para> <para>Half The application will present once per two refresh
	/// intervals (usually to preserve power). Much lag, no tearing.</para></param>
	void setVsyncMode(VsyncMode mode);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set a desired number of swap images (number of
	/// framebuffers). This number will be clamped between the minimum and the maximum number supported by the
	/// platform, so that if a small (0-1) or large (8+) number is requested, the minimum/maximum of the platform will
	/// always be providedd</summary>
	/// <param name="swapChainLength">The desired number of swap images. Default is 3 for Mailbox, 2 for On.</param>
	void setPreferredSwapChainLength(uint32 swapChainLength);

	/// <summary>EFFECTIVE IF CALLED DURING RenderFrame. Force the shell to ReleaseView and then InitView again after this
	/// frame.</summary>
	void forceReinitView();

	/// <summary>Get the number of anti-aliasing samples.</summary>
	/// <returns>The number of anti-aliasing samples.</returns>
	uint32 getAASamples() const;

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the Anti-Aliasing samples.</summary>
	/// <param name="value">The anti-aliasing samples.</param>
	void setAASamples(uint32 value);

	/// <returns>Get the total number of color bits per pixel (sum of all channels' bits per pixel)</returns>
	/// <returns>The number of total color bits per pixel.</returns>
	uint32 getColorBitsPerPixel() const;

	/// <summary>Get the number of framebuffer depth bits per pixel.</summary>
	/// <returns>The number of depth bits per pixel.</returns>
	uint32 getDepthBitsPerPixel() const;

	/// <summary>Get the number of framebuffer stencil bits per pixel.</summary>
	/// <returns>The number of stencil bits per pixel.</returns>
	uint32 getStencilBitsPerPixel() const;

	/// <summary>Get the Colorspace of the main window framebuffer (linear RGB or sRGB).</summary>
	/// <returns>The Colorspace of the main window framebuffer (linear RGB or sRGB).</returns>
	types::ColorSpace getBackBufferColorspace();

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Specify the colorspace of the backbuffer. Default is a
	/// (linear) RGB BackBuffer. Use this to specifically request an sRGB backbuffer. Since the support of backbuffer
	/// colorspace is an extension in many implementations, if you use this function, you must call
	/// getBackBufferColorspace after initApplication (in initView) to determine the actual backBuffer colorspace that
	/// was obtained.</summary>
	/// <param name="colorSpace">the desired framebuffer colorspace (either lRgb or sRgb)</param>
	void setBackBufferColorspace(types::ColorSpace colorSpace);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the number of framebuffer color bits per pixel.
	/// </summary>
	/// <param name="r">The Red framebuffer channel color bits</param>
	/// <param name="g">The Green framebuffer channel color bits</param>
	/// <param name="b">The Blue framebuffer channel color bits</param>
	/// <param name="a">The Alpha framebuffer channel color bits</param>
	/// <remarks>Actual number obtained may vary per implementation. Query with getColorBitsPerPixel after initView to
	/// check the actual number obtained</remarks>
	void setColorBitsPerPixel(uint32 r, uint32 g, uint32 b, uint32 a);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the number of framebuffer Depth bits per pixel.
	/// </summary>
	/// <param name="value">The desired framebuffer Depth channel bits.</param>
	/// <remarks>Actual number obtained may vary per implementation. Query with getDepthBitsPerPixel after initView to
	/// check the actual number obtained</remarks>
	void setDepthBitsPerPixel(uint32 value);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Set the number of Stencil bits per pixel.</summary>
	/// <param name="value">The Stencil channel bits.</param>
	/// <remarks>Actual number obtained may vary per implementation. Query with getStencilBitsPerPixel after initView to
	/// check the actual number obtained</remarks>
	void setStencilBitsPerPixel(uint32 value);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Forces frame time to always be reported as 1/60th of a
	/// second.</summary>
	/// <param name="value">True to force frame time, false to use real frame time.</param>
	void setForceFrameTime(const bool value);

	/// <summary>Check if frame time is forced (forceFrameTime(true) has been called)</summary>
	/// <returns>True if forcing frame time.</returns>
	bool isForcingFrameTime();

	/// <summary>Check if screen is rotated</summary>
	/// <returns>True if screen is Landscape (height > width), false otherwise</returns>
	bool isScreenRotated()const;

	/// <summary>return true if present backbuffer is enabled</summary>
	/// <returns>true if presenting backbuffer</returns>
	bool isPresentingBackBuffer();

	/// <summary>enable or disable presenting back-buffer</summary>
	/// <param name="value">Set to true to present, false to disable actual backbuffer swapping/presentation.</param>
	void setPresentBackBuffer(const bool value);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Sets a specific Graphics API type (version) that the
	/// user wants to use. The context creation will fail if this precise version cannot be created.</summary>
	/// <param name="contextType">The context type requested.</param>
	void setApiTypeRequired(Api contextType);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. ONLY EFFECTIVE IF PVRAPI IS LINKED DYNAMICALLY. If PVRApi
	/// is linked statically, this option is ignored. Sets a specific Graphics API type (version) that the
	/// user wants to use. The context creation will fail if this precise version cannot be created.</summary>
	/// <param name="contextType">The context type requested.</param>
	void setApiTypeBase(BaseApi contextType);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Sets the minimum Graphics API type (version) that the
	/// user intends to use. The context creation will fail if at least this context version cannot be created. The
	/// latest context version supported will be created.</summary>
	/// <param name="contextType">The context type requested.</param>
	void setMinApiType(Api contextType);

	/// <summary>Gets the minimum Graphics API type (version) that the user has set. See setMinApiType.</summary>
	/// <returns>The api type and version that the user has set as the minimum acceptable.</returns>
	Api getMinApiTypeRequired();

	/// <summary>Gets the maximum supported Graphics API type.</summary>
	/// <returns>The api type and maximum version that the implementation can provide.</returns>
	Api getMaxApiLevel();

	/// <summary>Queries if a particular Graphics API type/version is supported.</summary>
	/// <param name="api">The context type queried.</param>
	/// <returns>True if api is supported</returns>
	bool isApiSupported(Api api);

	/// <summary>Get the context type/version (if any) that will be requested</summary>
	/// <returns>The context type that will be requested from PVRApi.</returns>
	Api  getApiTypeRequired();

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT VIEW OR AFTER. Get the actual API version created.</summary>
	/// <returns>Return the actual API level of the existing Graphics Context. If called before initView, results are
	/// undefined.</returns>
	Api  getApiType()const;

	/// <summary>Get the context type/version (if any) that will be/was requested. Before init view, returns the
	/// requested base api type (OpenGL ES/ Vulkan). After init application, it returns the Created api type. This
	/// only makes sense if the PVRApi library is loaded Dynamically (in which case it controls the PVRApi library
	/// that the Shell will attempt to load, otherwise the Base api type is fixed to the linked-in library. Default:
	/// BaseApi::Unspecified. If unspecified, the application will try Vulkan first, then OpenGL ES, then will set
	/// it to whichever one succeeded first.</summary>
	/// <returns>The context type that will be requested from PVRApi, OR the context that was created (if it was
	/// unspecified.</returns>
	BaseApi  getApiTypeBase();

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Sets the Device Queue types that the user may require
	/// (Graphics, Compute etc.).</summary>
	/// <param name="queueType">The DeviceQueueType requested.</param>
	void setDeviceQueueTypesRequired(DeviceQueueType queueType);

	/// <summary>Get the DeviceQueueTypes that have been set as required.</summary>
	/// <returns>The DeviceQueueTypes that have been set as required.</returns>
	DeviceQueueType getDeviceQueueTypesRequired();

	/// <summary>Print out general information about this Shell (application name, sdk version, cmd line etc.</summary>
	void showOutputInfo();

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Captures the frames between start and stop and saves
	/// them as TGA screenshots.</summary>
	/// <param name="start">First frame to be captured</param>
	/// <param name="stop">Last frame to be captured</param>
	void setCaptureFrames(uint32 start, uint32 stop);

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. Sets the upscale factor of the screenshots. Upscaling
	/// only.</summary>
	/// <param name="value">Upscaling of the screenshots</param>
	void setCaptureFrameScale(uint32 value);

	/// <summary>If capturing frames, get the first frame to be captured.</summary>
	/// <returns>If capturing frames, the first frame to be captured.</returns>
	uint32 getCaptureFrameStart() const;

	/// <summary>If capturing frames, get the last frame to be captured.</summary>
	/// <returns>If capturing frames, the last frame to be captured.</returns>
	uint32 getCaptureFrameStop() const;

	/// <summary>Get the requested context priority.0=Low,1=Medium, 2+ = High. Initial value: High.</summary>
	/// <returns>If supported, the priority of the main Graphics Context used by the application.</returns>
	uint32 getContextPriority() const;

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. If supported, sets a ContextPriority that the shell will
	/// attempt to use when creating the main Graphics Context used for the window. Initial value:High.</summary>
	/// <param name="value">The context priority requested. 0=Low, 1=Medium, 2+=High.</param>
	void setContextPriority(uint32 value);

	/// <summary>If setDesiredConfig was called, get the desired ConfigID.</summary>
	/// <returns>If setDesiredConfig was called, the desired ConfigID.</returns>
	uint32 getDesiredConfig() const;

	/// <summary>ONLY EFFECTIVE IF CALLED AT INIT APPLICATION. If supported by the platform/API, sets a specific
	/// Context Configuration ID to be used.</summary>
	/// <param name="value">The ConfigID that will be requested.</param>
	void setDesiredConfig(uint32 value);

	/// <summary>Get the artificial frame time that has been set. 0 means unset.</summary>
	/// <returns>The artificial frame time that has been set. 0 means unset.</returns>
	uint32 getFakeFrameTime() const;

	/// <summary>Sets a time delta that will be used as frame time to increment the application clock instead of real
	/// time. This number will be returned as the frame time.</summary>
	/// <param name="value">The number of milliseconds of the frame.</param>
	void setFakeFrameTime(uint32 value);

	/// <summary>Check if FPS are being printed out.</summary>
	/// <returns>True if FPS are being printed out.</returns>
	bool isShowingFPS() const;

	/// <summary>Sets if the Frames Per Second are to be output periodically.</summary>
	/// <param name="showFPS">Set to true to output fps, false otherwise.</param>
	void setShowFPS(bool showFPS);

	/// <summary>Get an FPS calculation of the last frame.</summary>
	/// <returns>An Frames-Per-Second value calculated periodically by the application.</returns>
	float getFPS() const;

	/// <summary>Get the current version of the PowerVR SDK.</summary>
	/// <returns>The current version of the PowerVR SDK.</returns>
	static const char8* getSDKVersion() { return PVRSDK_BUILD; }

	/// <summary>Set a message to be displayed on application exit. Normally used to display critical error messages
	/// that might be missed if displayed as just logs.</summary>
	/// <param name="format">A printf-style format string</param>
	/// <param name="...">Printf-style variable arguments</param>
	void setExitMessage(const char8* const format, ...);

	/// <summary>Sets the application name.</summary>
	/// <param name="format">A printf-style format string</param>
	/// <param name="...">Printf-style variable parameters</param>
	void setApplicationName(const char8* const format, ...);

	/// <summary>Sets the window title. Will only be actually displayed If used on or before initApplication.
	/// </summary>
	/// <param name="format">A printf-style format string</param>
	/// <param name="...">Printf-style variable parameters</param>
	void setTitle(const char8* const format, ...);

	/// <summary>Get the exit message set by the user.</summary>
	/// <returns>The exit message set by the user.</returns>
	const string& getExitMessage() const;

	/// <summary>Get the window title.</summary>
	/// <returns>The window title.</returns>
	const string& getTitle() const;

	/// <summary>Get application name.</summary>
	/// <returns>The application name.</returns>
	const string& getApplicationName() const;

	/// <summary>Get the default read path.</summary>
	/// <returns>The first (default) read path. Normally, current directory.</returns>
	const string& getDefaultReadPath() const;

	/// <summary>Get a list of all paths that will be tried when looking for loading files.</summary>
	/// <returns>The a list of all the read paths that will successively be tried when looking to read a file.
	/// </returns>
	const std::vector<string>& getReadPaths() const;

	/// <summary>Get the path where any files will be saved.</summary>
	/// <returns>The path where any files saved (screenshots, logs) will be output to.</returns>
	const string& getWritePath() const;

	/// <summary>Signifies the application to clean up and exit. Will go through the normal StateMachine cycle and exit
	/// cleanly, exactly like returning ExitRenderFrame from RenderFrame. Will skip the next RenderFrame execution.
	/// </summary>
	void exitShell();

	/// <summary>Create and return a Stream object for a specific filename. Uses platform dependent lookup rules to
	/// create the stream from the filesystem or a platform-specific store (Windows resources, Android .apk assets)
	/// etc. Will first try the filesystem (if available) and then the built-in stores, in order to allow the user to
	/// easily override built-in assets.</summary>
	/// <param name="filename">The name of the file to load. Is usually a raw filename, but may contain a path.
	/// </param>
	/// <param name="logFileNotFound">Set this to false if file-not-found are expected and should not be logged as
	/// errors.</param>
	/// <returns>A unique pointer to the Stream returned if successful, an Empty unique pointer if failed.
	/// </returns>
	Stream::ptr_type getAssetStream(const string& filename, bool logFileNotFound = true);

	/// <summary>Gets the ShellOS object owned by this shell.</summary>
	/// <returns>The ShellOS object owned by this shell.</returns>
	ShellOS& getOS() const;

	/// <summary>Get the GraphicsContext utilized by this shell. If operating without a context, it will be empty.
	/// </summary>
	/// <returns>The GraphicsContext utilized by this shell.</returns>
	GraphicsContext& getGraphicsContext();

	/// <summary>Get the GraphicsContext utilized by this shell. If operating without a context, it will be empty.
	/// </summary>
	/// <returns>The GraphicsContext utilized by this shell.</returns>
	const GraphicsContext& getGraphicsContext() const;

	/// <summary>Get the GraphicsContext utilized by this shell. If operating without a context, it will be empty.
	/// </summary>
	/// <returns>The GraphicsContext utilized by this shell.</returns>
	GraphicsContext& context() { return getGraphicsContext(); }

	/// <summary>Get the GraphicsContext utilized by this shell. If operating without a context, it will be empty.
	/// </summary>
	/// <returns>The GraphicsContext utilized by this shell.</returns>
	const GraphicsContext& context() const { return getGraphicsContext(); }

	/// <summary>Get the Platform Context class used by this shell.</summary>
	/// <returns>The Platform Context class used by this shell.</returns>
	IPlatformContext& getPlatformContext();

	/// <summary>Get the Platform Context class used by this shell.</summary>
	/// <returns>The Platform Context class used by this shell.</returns>
	const IPlatformContext& getPlatformContext() const;

	/// <summary>Save a screenshot of the current display.</summary>
	void takeScreenshot() const;

	void prepareSharedContexts(const std::vector<SharedContextCapabilities>& contextList);

private:
	bool _dragging;
	std::bitset<256> _keystate;
	PrivatePointerState _pointerState;
	ShellData* _data;
	ConfigureEvent _configureEvent;
	SimplifiedInput MapKeyToMainInput(Keys key)
	{
		switch (key)
		{
		case Keys::Space: case Keys::Return: return SimplifiedInput::Action1;  break;
		case Keys::Escape: case Keys::Q: return SimplifiedInput::ActionClose; break;
		case Keys::Key1: return SimplifiedInput::Action2; break;
		case Keys::Key2: return SimplifiedInput::Action3; break;

		case Keys::Left: return SimplifiedInput::Left; break;
		case Keys::Right: return SimplifiedInput::Right; break;
		case Keys::Up: return SimplifiedInput::Up; break;
		case Keys::Down: return SimplifiedInput::Down; break;
		default: return SimplifiedInput::NONE;
		}
	}
	bool setKeyPressedVal(Keys key, char val)
	{
		bool temp = _keystate[(uint32)key];
		_keystate[(uint32)key] = (val != 0);
		return temp;
	}
	bool isKeyPressedVal(Keys key)
	{
		return _keystate[(uint32)key];
	}
	SimplifiedInput MapPointingDeviceButtonToSimpleInput(int buttonIdx)
	{
		switch (buttonIdx)
		{
		case 0: return SimplifiedInput::Action1;
		case 1: return SimplifiedInput::Action2;
		case 2: return SimplifiedInput::Action3;
		default: return SimplifiedInput::NONE;
		}
	}

};

}
using pvr::platform::Shell;
/// <summary>---IMPLEMENT THIS FUNCTION IN YOUR MAIN CODE FILE TO POWER YOUR APPLICATION---</summary>
/// <returns>The implementation of this function is typically a single, simple line: return
/// std::auto_ptr<pvr::Shell>(new MyApplicationClass());</returns>
/// <remarks>You must return an std::auto_ptr to pvr::Shell, that will be wrapping a pointer to an instance of
/// your Application class. The implementation is usually trivial.</remarks>
std::auto_ptr<pvr::Shell> newDemo();

#include "PVRShell/PVRShellDllImpls.h"
}
