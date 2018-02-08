/************************************************************************************

Filename    :   VrApi.h
Content     :   Minimum necessary API for mobile VR
Created     :   June 25, 2014
Authors     :   John Carmack, J.M.P. van Waveren
Language    :   C99

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

*************************************************************************************/
#ifndef OVR_VrApi_h
#define OVR_VrApi_h

#include "VrApi_Config.h"
#include "VrApi_Version.h"
#include "VrApi_Types.h"

/*

VrApi
=====

Multiple Android activities that live in the same address space can cooperatively use the VrApi.
However, only one activity can be in "VR mode" at a time. The following explains when an activity
is expected to enter/leave VR mode.


Android Activity life cycle
===========================

An Android Activity can only be in VR mode while the activity is in the resumed state.
The following shows how VR mode fits into the Android Activity life cycle.

	1.  VrActivity::onCreate() <---------+
	2.  VrActivity::onStart() <-------+  |
	3.  VrActivity::onResume() <---+  |  |
	4.  vrapi_EnterVrMode()        |  |  |
	5.  vrapi_LeaveVrMode()        |  |  |
	6.  VrActivity::onPause() -----+  |  |
	7.  VrActivity::onStop() ---------+  |
	8.  VrActivity::onDestroy() ---------+


Android Surface life cycle
==========================

An Android Activity can only be in VR mode while there is a valid Android Surface.
The following shows how VR mode fits into the Android Surface life cycle.

	1.  VrActivity::surfaceCreated() <----+
	2.  VrActivity::surfaceChanged()      |
	3.  vrapi_EnterVrMode()               |
	4.  vrapi_LeaveVrMode()               |
	5.  VrActivity::surfaceDestroyed() ---+

Note that the life cycle of a surface is not necessarily tightly coupled with the
life cycle of an activity. These two life cycles may interleave in complex ways.
Usually surfaceCreated() is called after onResume() and surfaceDestroyed() is called
between onPause() and onDestroy(). However, this is not guaranteed and, for instance,
surfaceDestroyed() may be called after onDestroy() or even before onPause().

An Android Activity is only in the resumed state with a valid Android Surface between
surfaceChanged() or onResume(), whichever comes last, and surfaceDestroyed() or onPause(),
whichever comes first. In other words, a VR application will typically enter VR mode
from surfaceChanged() or onResume(), whichever comes last, and leave VR mode from
surfaceDestroyed() or onPause(), whichever comes first.


Android VR life cycle
=====================

// Setup the Java references.
ovrJava java;
java.Vm = javaVm;
java.Env = jniEnv;
java.ActivityObject = activityObject;

// Initialize the API.
const ovrInitParms initParms = vrapi_DefaultInitParms( &java );
if ( vrapi_Initialize( &initParms ) != VRAPI_INITIALIZE_SUCCESS )
{
	FAIL( "Failed to initialize VrApi!" );
	abort();
}

// Create an EGLContext for the application.
EGLContext eglContext = ;	// application's context

// Get the suggested resolution to create eye texture swap chains.
const int suggestedEyeTextureWidth = vrapi_GetSystemPropertyInt( &java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH );
const int suggestedEyeTextureHeight = vrapi_GetSystemPropertyInt( &java, VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT );

// Allocate a texture swap chain for each eye with the application's EGLContext current.
ovrTextureSwapChain * colorTextureSwapChain[VRAPI_FRAME_LAYER_EYE_MAX];
for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
{
	colorTextureSwapChain[eye] = vrapi_CreateTextureSwapChain( VRAPI_TEXTURE_TYPE_2D, VRAPI_TEXTURE_FORMAT_8888,
																suggestedEyeTextureWidth,
																suggestedEyeTextureHeight,
																1, true );
}

// Android Activity/Surface life cycle loop.
while ( !exit )
{
	// Acquire ANativeWindow from Android Surface.

	ANativeWindow * nativeWindow = ;	// ANativeWindow for the Android Surface
	bool resumed = ;					// set to true in onResume() and set to false in onPause()

	while ( resumed && nativeWindow != NULL )
	{
		// Enter VR mode once the Android Activity is in the resumed state with a valid ANativeWindow.
		ovrModeParms modeParms = vrapi_DefaultModeParms( &java );
		modeParms.Flags |= VRAPI_MODE_FLAG_NATIVE_WINDOW;
		modeParms.Display = eglDisplay;
		modeParms.WindowSurface = nativeWindow;
		modeParms.ShareContext = eglContext;
		ovrMobile * ovr = vrapi_EnterVrMode( &modeParms );

		// Set the tracking transform to use, by default this is eye level.
		vrapi_SetTrackingTransform( ovr, vrapi_GetTrackingTransform( ovr, VRAPI_TRACKING_TRANSFORM_SYSTEM_CENTER_EYE_LEVEL ) );

		// Frame loop, possibly running on another thread.
		for ( long long frameIndex = 1; resumed && nativeWindow != NULL; frameIndex++ )
		{
			// Get the HMD pose, predicted for the middle of the time period during which
			// the new eye images will be displayed. The number of frames predicted ahead
			// depends on the pipeline depth of the engine and the synthesis rate.
			// The better the prediction, the less black will be pulled in at the edges.
			const double predictedDisplayTime = vrapi_GetPredictedDisplayTime( ovr, frameIndex );
			const ovrTracking2 tracking = vrapi_GetPredictedTracking2( ovr, predictedDisplayTime );

			// Advance the simulation based on the predicted display time.

			// Render eye images and setup the 'ovrSubmitFrameDesc2' using 'ovrTracking2' data.

			ovrLayerProjection2 layer = vrapi_DefaultLayerProjection2();
			layer.HeadPose = tracking.HeadPose;
			for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
			{
				const int colorTextureSwapChainIndex = frameIndex % vrapi_GetTextureSwapChainLength( colorTextureSwapChain[eye] );
				const unsigned int textureId = vrapi_GetTextureSwapChainHandle( colorTextureSwapChain[eye], colorTextureSwapChainIndex );

				// Render to 'textureId' using the 'ProjectionMatrix' from 'ovrTracking2'.

				layer.Textures[eye].ColorSwapChain = colorTextureSwapChain[eye];
				layer.Textures[eye].SwapChainIndex = colorTextureSwapChainIndex;
				layer.Textures[eye].TexCoordsFromTanAngles = ovrMatrix4f_TanAngleMatrixFromProjection( &tracking.Eye[eye].ProjectionMatrix );
			}

			// Insert 'fence' using eglCreateSyncKHR.

			const ovrLayerHeader2 * layers[] =
			{
				&layer.Header
			};

			ovrSubmitFrameDescription2 frameDesc = {};
			frameDesc.FrameIndex = frameIndex;
			frameDesc.DisplayTime = predictedDisplayTime;
			frameDesc.CompletionFence = fence;
			frameDesc.LayerCount = 1;
			frameDesc.Layers = layers;

			// Hand over the eye images to the time warp.
			vrapi_SubmitFrame2( ovr, &frameDesc );
		}
	}

	// Must leave VR mode when the Android Activity is paused or the Android Surface is destroyed or changed.
	vrapi_LeaveVrMode( ovr );
}

// Destroy the texture swap chains.
// Make sure to delete the swapchains before the application's EGLContext is destroyed.
for ( int eye = 0; eye < VRAPI_FRAME_LAYER_EYE_MAX; eye++ )
{
	vrapi_DestroyTextureSwapChain( colorTextureSwapChain[eye] );
}

// Shut down the API.
vrapi_Shutdown();


Integration
===========

The API is designed to work with an Android Activity using a plain Android SurfaceView,
where the Activity life cycle and the Surface life cycle are managed completely in native
code by sending the life cycle events (onResume, onPause, surfaceChanged etc.) to native code.

The API does not work with an Android Activity using a GLSurfaceView. The GLSurfaceView
class manages the window surface and EGLSurface and the implementation of GLSurfaceView
may unbind the EGLSurface before onPause() gets called. As such, there is no way to
leave VR mode before the EGLSurface disappears. Another problem with GLSurfaceView is
that it creates the EGLContext using eglChooseConfig(). The Android EGL code pushes in
multisample flags in eglChooseConfig() if the user has selected the "force 4x MSAA" option
in settings. Using a multisampled front buffer is completely wasted for time warp
rendering.

Alternatively an Android NativeActivity can be used to avoid manually handling all
the life cycle events. However, it is important to select the EGLConfig manually
without using eglChooseConfig() to make sure the front buffer is not multisampled.

The vrapi_GetSystemProperty* functions can be called at any time from any thread.
This allows an application to setup its renderer, possibly running on a separate
thread, before entering VR mode.

On Android, an application cannot just allocate a new window/frontbuffer and render to it.
Android allocates and manages the window/frontbuffer and (after the fact) notifies the
application of the state of affairs through life cycle events (surfaceCreated /
surfaceChanged / surfaceDestroyed). The application (or 3rd party engine) typically handles
these events. Since the VrApi cannot just allocate a new window/frontbuffer, and the VrApi
does not handle the life cycle events, the VrApi somehow has to take over ownership of the
Android surface from the application. To allow this, the application can explicitly pass
the EGLDisplay, EGLContext, EGLSurface, or ANativeWindow to vrapi_EnterVrMode(), where the
EGLSurface is the surface created from the ANativeWindow. The EGLContext is used to match
the version and config for the context used by the background time warp thread. This EGLContext,
and no other context can be current on the EGLSurface.

If, however, the application does not explicitly pass in these objects, then vrapi_EnterVrMode()
*must* be called from a thread with an OpenGL ES context current on the Android window surface.
The context of the calling thread is then used to match the version and config for the context
used by the background time warp thread. The time warp will also hijack the Android window surface
from the context that is current on the calling thread. On return, the context from the calling
thread will be current on an invisible pbuffer, because the time warp takes ownership of the
Android window surface. Note that this requires the config used by the calling thread to have
an EGL_SURFACE_TYPE with EGL_PBUFFER_BIT.

Sensor input only becomes available after entering VR mode. In part this is because the
VrApi supports hybrid apps. The app starts out in non-stereo mode, and only switches to
VR mode when the phone is docked into the headset. While not in VR mode, a non-stereo app
should not be burdened with a SCHED_FIFO device manager thread for sensor input and possibly
expensive sensor/vision processing. In other words, there is no sensor input until the
phone is docked and the app is in VR mode.

Before getting sensor input, the application also needs to know when the images that are
going to be synthesized will be displayed, because the sensor input needs to be predicted
ahead for that time. As it turns out, it is not trivial to get an accurate predicted
display time. Therefore the calculation of this predicted display time is part of the VrApi.
An accurate predicted display time can only really be calculated once the rendering loop
is up and running and submitting frames regularly. In other words, before getting sensor
input, the application needs an accurate predicted display time, which in return requires
the renderer to be up and running. As such, it makes sense that sensor input is not
available until vrapi_EnterVrMode() has been called. However, once the application is
in VR mode, it can call vrapi_GetPredictedDisplayTime() and vrapi_GetPredictedTracking()
at any time from any thread.

The VrApi allows for one frame of overlap which is essential on tiled mobile GPUs. Because
there is one frame of overlap, the eye images have typically not completed rendering by the
time they are submitted to vrapi_SubmitFrame(). To allow the time warp to check whether the
eye images have completed rendering, the application can explicitly pass in a sync object
(CompletionFence) for each eye image through vrapi_SubmitFrame(). Note that these sync
objects must be EGLSyncKHR because the VrApi still supports OpenGL ES 2.0.

If, however, the application does not explicitly pass in sync objects, then vrapi_SubmitFrame()
*must* be called from the thread with the OpenGL ES context that was used for rendering,
which allows vrapi_SubmitFrame() to add a sync object to the current context and check
if rendering has completed.

Note that even if no OpenGL ES objects are explicitly passed through the VrApi, then
vrapi_EnterVrMode() and vrapi_SubmitFrame() can still be called from different threads.
vrapi_EnterVrMode() needs to be called from a thread with an OpenGL ES context that is current
on the Android window surface. This does not need to be the same context that is also used
for rendering. vrapi_SubmitFrame() needs to be called from the thread with the OpenGL ES
context that was used to render the eye images. If this is a different context than the context
used to enter VR mode, then for stereoscopic rendering this context *never* needs to be current
on the Android window surface.

Eye images are passed to vrapi_SubmitFrame() as "texture swap chains" (ovrTextureSwapChain).
These texture swap chains are allocated through vrapi_CreateTextureSwapChain(). This is
important to allow these textures to be allocated in special system memory. When using
a static eye image, the texture swap chain does not need to be buffered and the chain
only needs to hold a single texture. When the eye images are dynamically updated, the
texture swap chain needs to be buffered. When the texture swap chain is passed to
vrapi_SubmitFrame(), the application also passes in the chain index to the most recently
updated texture.


Frame Timing
============

vrapi_SubmitFrame() controls the synthesis rate through an application specified
frame parameter, SwapInterval. vrapi_SubmitFrame() also controls at which point during
a display refresh cycle the calling thread gets released. vrapi_SubmitFrame() only returns
when the previous eye images have been consumed by the asynchronous time warp thread,
and at least the specified minimum number of V-syncs have passed since the last call
to vrapi_SubmitFrame(). The asynchronous time warp thread consumes new eye images and
updates the V-sync counter halfway through a display refresh cycle. This is the first
time the time warp can start updating the first eye, covering the first half of the
display. As a result, vrapi_SubmitFrame() returns and releases the calling thread halfway
through a display refresh cycle.

Once vrapi_SubmitFrame() returns, synthesis has a full display refresh cycle to generate
new eye images up to the next halfway point. At the next halfway point, the time
warp has half a display refresh cycle (up to V-sync) to update the first eye. The
time warp then effectively waits for V-sync and then has another half a display
refresh cycle (up to the next-next halfway point) to update the second eye. The
asynchronous time warp uses a high priority GPU context and will eat away cycles
from synthesis, so synthesis does not have a full display refresh cycle worth of
actual GPU cycles. However, the asynchronous time warp tends to be very fast,
leaving most of the GPU time for synthesis.

Instead of just using the latest sensor sampling, synthesis uses predicted sensor input
for the middle of the time period during which the new eye images will be displayed.
This predicted time is calculated using vrapi_GetPredictedDisplayTime(). The number
of frames predicted ahead depends on the pipeline depth, the extra latency mode, and
the minimum number of V-syncs in between eye image rendering. Less than half a display
refresh cycle before each eye image will be displayed, the asynchronous time warp will
get new predicted sensor input using the very latest sensor sampling. The asynchronous
time warp then corrects the eye images using this new sensor input. In other words,
the asynchronous time warp will always correct the eye images even if the predicted
sensor input for synthesis was not perfect. However, the better the prediction for
synthesis, the less black will be pulled in at the edges by the asynchronous time warp.

The application can improve the prediction by fetching the latest predicted sensor
input right before rendering each eye, and passing a, possibly different, sensor state
for each eye to vrapi_SubmitFrame(). However, it is very important that both eyes use a
sensor state that is predicted for the exact same display time, so both eyes can be
displayed at the same time without causing intra frame motion judder. While the predicted
orientation can be updated for each eye, the position must remain the same for both eyes,
or the position would seem to judder "backwards in time" if a frame is dropped.

Ideally the eye images are only displayed for the SwapInterval display refresh cycles
that are centered about the eye image predicted display time. In other words, a set
of eye images is first displayed at prediction time minus SwapInterval / 2 display
refresh cycles. The eye images should never be shown before this time because that
can cause intra frame motion judder. Ideally the eye images are also not shown after
the prediction time plus SwapInterval / 2 display refresh cycles, but this may
happen if synthesis fails to produce new eye images in time.

SwapInterval = 1
ExtraLatencyMode = off
Expected single-threaded simulation latency = 33 milliseconds
The ATW brings this down to 8-16 milliseconds.
|-------|-------|-------|  - V-syncs
|   *   |   *   |   *   |  - eye image display periods (* = predicted time in middle of display period)
     \     / \ / \ /
    ^ \   / ^ |   +---- The asynchronous time warp projects the second eye image onto the display.
    |  \ /  | +---- The asynchronous time warp projects the first eye image onto the display. 
    |   |   |
    |   |   +---- Call vrapi_SubmitFrame before this point.
    |   |         vrapi_SubmitFrame inserts a GPU fence and hands over eye images to the asynchronous time warp.
    |   |         The asynchronous time warp checks the fence and uses the new eye images if rendering has completed.
    |   |
    |   +---- Generate GPU commands and execute commands on GPU.
    |
    +---- vrapi_SubmitFrame releases the renderer thread.

SwapInterval = 1
ExtraLatencyMode = on
Expected single-threaded simulation latency = 49 milliseconds
The ATW brings this down to 8-16 milliseconds.
|-------|-------|-------|-------|  - V-syncs
|   *   |   *   |   *   |   *   |  - display periods (* = predicted time in middle of display period)
     \             / \ / \ /
    ^ \           / ^ |   +---- The asynchronous time warp projects second eye image onto the display.
    |  \         /  | +---- The asynchronous time warp projects first eye image onto the display. 
    |   \       /   |
    |    \     /    +---- Submit frame before this point.
    |     \   /           Frame submission inserts a GPU fence and hands over eye textures
    |      \ /            to the asynchronous time warp. The asynchronous time warp checks
    |       |             the fence and uses the new eye textures if rendering has completed.
    |       |
    |       +---- Generate GPU commands on CPU and execute commands on GPU.
    |
    +---- Frame submission releases the renderer thread.

SwapInterval = 2
ExtraLatencyMode = off
Expected single-threaded simulation latency = 58 milliseconds
The ATW brings this down to 8-16 milliseconds.
|-------|-------|-------|-------|-------|  - V-syncs
*       |       *       |       *       |  - eye image display periods (* = predicted time in middle of display period)
     \             / \ / \ / \ / \ /
    ^ \           / ^ |   |   |   +---- The asynchronous time warp re-projects the second eye image onto the display.
    |  \         /  | |   |   +---- The asynchronous time warp re-projects the first eye image onto the display. 
    |   \       /   | |   +---- The asynchronous time warp projects the second eye image onto the display.
    |    \     /    | +---- The asynchronous time warp projects the first eye image onto the display.
    |     \   /     |
    |      \ /      +---- Call vrapi_SubmitFrame before this point.
    |       |             vrapi_SubmitFrame inserts a GPU fence and hands over eye images to the asynchronous time warp.
    |       |             The asynchronous time warp checks the fence and uses the new eye images if rendering has completed.
    |       |
    |       +---- Generate GPU commands and execute commands on GPU.
    |
    +---- vrapi_SubmitFrame releases the renderer thread.

SwapInterval = 2
ExtraLatencyMode = on
Expected single-threaded simulation latency = 91 milliseconds
The ATW brings this down to 8-16 milliseconds.
|-------|-------|-------|-------|-------|-------|-------|  - V-syncs
*       |       *       |       *       |       *       |  - eye image display periods (* = predicted time in middle of display period)
     \                             / \ / \ / \ / \ /
    ^ \                           / ^ |   |   |   +---- The asynchronous time warp re-projects the second eye image onto the display.
    |  \                         /  | |   |   +---- The asynchronous time warp re-projects the first eye image onto the display. 
    |   \                       /   | |   +---- The asynchronous time warp projects the second eye image onto the display.
    |    \                     /    | +---- The asynchronous time warp projects the first eye image onto the display.
    |     \                   /     |
    |      \                 /      +---- Call vrapi_SubmitFrame before this point.
    |       \               /             vrapi_SubmitFrame inserts a GPU fence and hands over eye images to the asynchronous time warp.
    |        +------+------+              The asynchronous time warp checks the fence and uses the new eye images if rendering has completed.
    |               |
    |               +---- Generate GPU commands and execute commands on GPU.
    |                
    +---- vrapi_SubmitFrame releases the renderer thread.

*/

#if defined( __cplusplus )
extern "C" {
#endif

// Returns the version + compile time stamp as a string.
// Can be called any time from any thread.
OVR_VRAPI_EXPORT const char * vrapi_GetVersionString();

// Returns global, absolute high-resolution time in seconds. This is the same value
// as used in sensor messages and on Android also the same as Java's system.nanoTime(),
// which is what the Choreographer V-sync timestamp is based on.
// WARNING: do not use this time as a seed for simulations, animations or other logic.
// An animation, for instance, should not be updated based on the "real time" the
// animation code is executed. Instead, an animation should be updated based on the
// time it will be displayed. Using the "real time" will introduce intra-frame motion
// judder when the code is not executed at a consistent point in time every frame.
// In other words, for simulations, animations and other logic use the time returned
// by vrapi_GetPredictedDisplayTime().
// Can be called any time from any thread.
OVR_VRAPI_EXPORT double vrapi_GetTimeInSeconds();

//-----------------------------------------------------------------
// Initialization/Shutdown
//-----------------------------------------------------------------

// Initializes the API for application use.
// This is lightweight and does not create any threads.
// This is typically called from onCreate() or shortly thereafter.
// Can be called from any thread.
// Returns a non-zero value from ovrInitializeStatus on error.
OVR_VRAPI_EXPORT ovrInitializeStatus vrapi_Initialize( const ovrInitParms * initParms );

// Shuts down the API on application exit.
// This is typically called from onDestroy() or shortly thereafter.
// Can be called from any thread.
OVR_VRAPI_EXPORT void vrapi_Shutdown();


//-----------------------------------------------------------------
// System Properties
//-----------------------------------------------------------------

// Returns a system property. These are constants for a particular device.
// This function can be called any time from any thread once the VrApi is initialized.
OVR_VRAPI_EXPORT int vrapi_GetSystemPropertyInt( const ovrJava * java, const ovrSystemProperty propType );
OVR_VRAPI_EXPORT float vrapi_GetSystemPropertyFloat( const ovrJava * java, const ovrSystemProperty propType );
// The return memory is guaranteed to be valid until the next call to vrapi_GetSystemPropertyString.
OVR_VRAPI_EXPORT const char * vrapi_GetSystemPropertyString( const ovrJava * java, const ovrSystemProperty propType );

//-----------------------------------------------------------------
// System Status
//-----------------------------------------------------------------

// Returns a system status. These are variables that may change at run-time.
// This function can be called any time from any thread once the VrApi is initialized.
OVR_VRAPI_EXPORT int vrapi_GetSystemStatusInt( const ovrJava * java, const ovrSystemStatus statusType );
OVR_VRAPI_EXPORT float vrapi_GetSystemStatusFloat( const ovrJava * java, const ovrSystemStatus statusType );

//-----------------------------------------------------------------
// Enter/Leave VR mode
//-----------------------------------------------------------------

// Starts up the time warp, V-sync tracking, sensor reading, clock locking,
// thread scheduling, and sets video options. The parms are copied, and are
// not referenced after the function returns.
//
// This should be called after vrapi_Initialize(), when the app is both
// resumed and has a valid window surface (ANativeWindow).
//
// On Android, an application cannot just allocate a new window surface
// and render to it. Android allocates and manages the window surface and
// (after the fact) notifies the application of the state of affairs through
// life cycle events (surfaceCreated / surfaceChanged / surfaceDestroyed).
// The application (or 3rd party engine) typically handles these events.
// Since the VrApi cannot just allocate a new window surface, and the VrApi
// does not handle the life cycle events, the VrApi somehow has to take over
// ownership of the Android surface from the application. To allow this, the
// application can explicitly pass the EGLDisplay, EGLContext and EGLSurface
// or ANativeWindow to vrapi_EnterVrMode(). The EGLDisplay and EGLContext are
// used to create a shared context used by the background time warp thread.
//
// If, however, the application does not explicitly pass in these objects, then
// vrapi_EnterVrMode() *must* be called from a thread with an OpenGL ES context
// current on the Android window surface. The context of the calling thread is
// then used to match the version and config for the context used by the background
// time warp thread. The time warp will also hijack the Android window surface
// from the context that is current on the calling thread. On return, the context
// from the calling thread will be current on an invisible pbuffer, because the
// time warp takes ownership of the Android window surface. Note that this requires
// the config used by the calling thread to have an EGL_SURFACE_TYPE with EGL_PBUFFER_BIT.
//
// New applications should always explicitly pass in the EGLDisplay, EGLContext
// and ANativeWindow. The other paths are still available to support old applications
// but they will be deprecated.
//
// This function will return NULL when entering VR mode failed because the ANativeWindow
// was not valid. If the ANativeWindow's buffer queue is abandoned
// ("BufferQueueProducer: BufferQueue has been abandoned"), then the app can wait for a
// new ANativeWindow (through SurfaceCreated). If another API is already connected to
// the ANativeWindow ("BufferQueueProducer: already connected"), then the app has to first
// disconnect whatever is connected to the ANativeWindow (typically an EGLSurface).
OVR_VRAPI_EXPORT ovrMobile * vrapi_EnterVrMode( const ovrModeParms * parms );

// Shut everything down for window destruction or when the activity is paused.
// The ovrMobile object is freed by this function.
//
// Must be called from the same thread that called vrapi_EnterVrMode(). If the
// application did not explicitly pass in the Android window surface, then this
// thread *must* have the same OpenGL ES context that was current on the Android
// window surface before calling vrapi_EnterVrMode(). By calling this function,
// the time warp gives up ownership of the Android window surface, and on return,
// the context from the calling thread will be current again on the Android window
// surface.
OVR_VRAPI_EXPORT void vrapi_LeaveVrMode( ovrMobile * ovr );

//-----------------------------------------------------------------
// Tracking
//-----------------------------------------------------------------

// Returns a predicted absolute system time in seconds at which the next set
// of eye images will be displayed.
//
// The predicted time is the middle of the time period during which the new
// eye images will be displayed. The number of frames predicted ahead depends
// on the pipeline depth of the engine and the minumum number of V-syncs in
// between eye image rendering. The better the prediction, the less black will
// be pulled in at the edges by the time warp.
//
// The frameIndex is an application controlled number that uniquely identifies
// the new set of eye images for which synthesis is about to start. This same
// frameIndex must be passed to vrapi_SubmitFrame() when the new eye images are
// submitted to the time warp. The frameIndex is expected to be incremented
// once every frame before calling this function.
//
// Can be called from any thread while in VR mode.
OVR_VRAPI_EXPORT double vrapi_GetPredictedDisplayTime( ovrMobile * ovr, long long frameIndex );

// Returns the predicted sensor state based on the specified absolute system time
// in seconds. Pass absTime value of 0.0 to request the most recent sensor reading.
//
// Can be called from any thread while in VR mode.
OVR_VRAPI_EXPORT ovrTracking2 vrapi_GetPredictedTracking2( ovrMobile * ovr, double absTimeInSeconds );

OVR_VRAPI_EXPORT ovrTracking vrapi_GetPredictedTracking( ovrMobile * ovr, double absTimeInSeconds );

// Recenters the orientation on the yaw axis and will recenter the position
// when position tracking is available.
//
// Note that this immediately affects vrapi_GetPredictedTracking() which may
// be called asynchronously from the time warp. It is therefore best to
// make sure the screen is black before recentering to avoid previous eye
// images from being abrubtly warped across the screen.
//
// Can be called from any thread while in VR mode.
OVR_VRAPI_EXPORT void vrapi_RecenterPose( ovrMobile * ovr );


//-----------------------------------------------------------------
// Tracking Transform
//
//-----------------------------------------------------------------

// The coordinate system used by the tracking system is defined in meters
// with its positive y axis pointing up, but its origin and yaw are unspecified.
//
// Applications generally prefer to operate in a well-defined coordinate system
// relative to some base pose. The tracking transform allows the application to
// specify the space that tracking poses are reported in.
//
// The tracking transform is specified as the ovrPosef of the base pose in tracking
// system coordinates.
//
// Head poses the application supplies in vrapi_SubmitFrame() are transformed
// by the tracking transform.
// Pose predictions generated by the system are transformed by the inverse of the
// tracking transform before being reported to the application.
//
// Note that this immediately affects vrapi_SubmitFrame() and
// vrapi_GetPredictedTracking(). It is important for the app to make sure that
// the tracking transform does not change between the fetching of a pose prediction
// and the submission of poses in vrapi_SubmitFrame().
//
// The default Tracking Transform is VRAPI_TRACKING_TRANSFORM_SYSTEM_CENTER_EYE_LEVEL.


// Returns a pose suitable to use as a tracking transform.
// Applications that want to use an eye-level based coordinate system can fetch
// the VRAPI_TRACKING_TRANSFORM_SYSTEM_CENTER_EYE_LEVEL transform.
// Applications that want to use a floor-level based coordinate system can fetch
// the VRAPI_TRACKING_TRANSFORM_SYSTEM_CENTER_FLOOR_LEVEL transform.
// To determine the current tracking transform, applications can fetch the
// VRAPI_TRACKING_TRANSFORM_CURRENT transform.
OVR_VRAPI_EXPORT ovrPosef  vrapi_GetTrackingTransform( ovrMobile * ovr, ovrTrackingTransform whichTransform );

// Sets the transform used convert between tracking coordinates and a canonical
// application-defined space.
// Only the yaw component of the orientation is used.
OVR_VRAPI_EXPORT void vrapi_SetTrackingTransform( ovrMobile * ovr, ovrPosef pose );


//-----------------------------------------------------------------
// Texture Swap Chains
//
//-----------------------------------------------------------------

// Create a texture swap chain that can be passed to vrapi_SubmitFrame.
// Must be called from a thread with a valid OpenGL ES context current.
//
// Specifying 0 levels allows the individual texture ids to be set with
// vrapi_SetTextureSwapChainHandle().
//
// Specifying VRAPI_TEXTURE_SWAPCHAIN_FULL_MIP_CHAIN levels will calculate
// the levels based on width and height.
//
// Buffers used to be a bool that selected either a single texture index
// or a triple buffered index, but the new entry point allows up to 16 buffers
// to be allocated, which is useful for maintaining a deep video buffer queue
// to get better frame timing.
OVR_VRAPI_EXPORT ovrTextureSwapChain * vrapi_CreateTextureSwapChain2( ovrTextureType type, ovrTextureFormat format,
																int width, int height, int levels, int bufferCount );

OVR_VRAPI_EXPORT ovrTextureSwapChain * vrapi_CreateTextureSwapChain( ovrTextureType type, ovrTextureFormat format,
                                                                int width, int height, int levels, bool buffered );

// Destroy the given texture swap chain.
// Must be called from a thread with the same OpenGL ES context current when vrapi_CreateTextureSwapChain was called.
OVR_VRAPI_EXPORT void vrapi_DestroyTextureSwapChain( ovrTextureSwapChain * chain );

// Returns the number of textures in the swap chain.
OVR_VRAPI_EXPORT int vrapi_GetTextureSwapChainLength( ovrTextureSwapChain * chain );

// Get the OpenGL name of the texture at the given index.
OVR_VRAPI_EXPORT unsigned int vrapi_GetTextureSwapChainHandle( ovrTextureSwapChain * chain, int index );

// Set the OpenGL name of the texture at the given index. NOTE: This is not portable to PC.
// This will silently fail with:
// W/TimeWarp: SetTextureSwapChainHandle: chain->Allocated
// Unless the SwapChain was created with 0 levels.
OVR_VRAPI_EXPORT void vrapi_SetTextureSwapChainHandle( ovrTextureSwapChain * chain, int index, unsigned int handle );

//-----------------------------------------------------------------
// Frame Submission
//-----------------------------------------------------------------

// Accepts new eye images plus poses that will be used for future warps.
// The parms are copied, and are not referenced after the function returns.
//
// This will block until the textures from the previous vrapi_SubmitFrame() have been
// consumed by the background thread, to allow one frame of overlap for maximum
// GPU utilization, while preventing multiple frames from piling up variable latency.
//
// This will block until at least SwapInterval vsyncs have passed since the last
// call to vrapi_SubmitFrame() to prevent applications with simple scenes from
// generating completely wasted frames.
//
// IMPORTANT: any dynamic textures that are passed to vrapi_SubmitFrame() must be
// triple buffered to avoid flickering and performance problems.
//
// The VrApi allows for one frame of overlap which is essential on tiled mobile GPUs.
// Because there is one frame of overlap, the eye images have typically not completed
// rendering by the time they are submitted to vrapi_SubmitFrame(). To allow the time
// warp to check whether the eye images have completed rendering, the application can
// explicitly pass in a sync object (CompletionFence) for each eye image through
// vrapi_SubmitFrame(). Note that these sync objects must be EGLSyncKHR because the
// VrApi still supports OpenGL ES 2.0.
// 
// If, however, the application does not explicitly pass in sync objects, then
// vrapi_SubmitFrame() *must* be called from the thread with the OpenGL ES context that
// was used for rendering, which allows vrapi_SubmitFrame() to add a sync object to
// the current context and check if rendering has completed.
OVR_VRAPI_EXPORT void vrapi_SubmitFrame( ovrMobile * ovr, const ovrFrameParms * parms );

// vrapi_SubmitFrame2 takes a frameDescription describing per-frame information such as:
// a flexible list of layers which should be drawn this frame, a single fence to signal
// frame completion, and a frame index.
OVR_VRAPI_EXPORT ovrResult vrapi_SubmitFrame2( ovrMobile * ovr, const ovrSubmitFrameDescription2 * frameDescription );

//-----------------------------------------------------------------
// Performance
//-----------------------------------------------------------------

// Set the CPU and GPU performance levels.
//
// Increasing the levels increases performance at the cost of higher power consumption
// which likely leads to a greater chance of overheating.
//
// Levels will be clamped to the expected range. Default clock levels are cpuLevel = 2, gpuLevel = 2.
OVR_VRAPI_EXPORT ovrResult vrapi_SetClockLevels( ovrMobile * ovr, const int32_t cpuLevel, const int32_t gpuLevel );

// Specify which app threads should be given higher scheduling priority.
OVR_VRAPI_EXPORT ovrResult vrapi_SetPerfThread( ovrMobile * ovr, const ovrPerfThreadType type, const uint32_t threadId );

// If VRAPI_EXTRA_LATENCY_MODE_ON specified, adds an extra frame of latency for full GPU utilization.
// Default is VRAPI_EXTRA_LATENCY_MODE_OFF.
// 
// The latency mode specified will be applied on the next call to vrapi_SubmitFrame(2).
OVR_VRAPI_EXPORT ovrResult vrapi_SetExtraLatencyMode( ovrMobile * ovr, const ovrExtraLatencyMode mode );

#if defined( __cplusplus )
}	// extern "C"
#endif

#endif	// OVR_VrApi_h
