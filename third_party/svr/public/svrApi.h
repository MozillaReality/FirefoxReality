//=============================================================================
//! \file svrApi.h
//
//                  Copyright (c) 2015 QUALCOMM Technologies Inc.
//                              All Rights Reserved.
//
//==============================================================================

#ifndef _SVR_API_H_
#define _SVR_API_H_

#include <stdlib.h>
#include <jni.h>

#define SVR_MAJOR_VERSION       2
#define SVR_MINOR_VERSION       1
#define SVR_REVISION_VERSION    1

#define SVR_MAX_EYE_LAYERS      8
#define SVR_MAX_OVERLAY_LAYERS  8


enum SvrResult
{
    // Return code for success
    SVR_ERROR_NONE = 0,

    // Return code for cases that don't fall into defined error codes
    SVR_ERROR_UNKNOWN = 1,

    // Return code to indicate that SVR isn't supported, or the necessary functionality in the system drivers
    // or VR service isn't supported.
    SVR_ERROR_UNSUPPORTED,

    // Return code for calls to functions made without first calling svrInitialize
    SVR_ERROR_VRMODE_NOT_INITIALIZED,

    // Return code for calls made into functions that require VR mode to be in a started state
    SVR_ERROR_VRMODE_NOT_STARTED,

    // Return code for calls made into functions that require VR mode to be in a stopped state
    SVR_ERROR_VRMODE_NOT_STOPPED,

    // Return code for calls made into functions where the service fails or isn't accessible
    SVR_ERROR_QVR_SERVICE_UNAVAILABLE,

    // Error for any failures in JNI/Java calls made through the API
    SVR_ERROR_JAVA_ERROR,
};

//! \brief Events
enum svrEventType
{
    kEventNone = 0,
    kEventSdkServiceStarting = 1,
    kEventSdkServiceStarted = 2,
    kEventSdkServiceStopped = 3,
    kEventControllerConnecting = 4,
    kEventControllerConnected = 5,
    kEventControllerDisconnected = 6,
    kEventThermal = 7,
    kEventVrModeStarted = 8,
    kEventVrModeStopping = 9,
    kEventVrModeStopped = 10,
    kEventSensorError = 11,
    kEventMagnometerUncalibrated
};

struct ANativeWindow;

//! \brief Simple structure to hold 2-component vector data
struct svrVector2
{
    float x,y;
};

//! \brief Simple structure to hold 3-component vector data
struct svrVector3
{
    float x,y,z;
};

//! \brief Simple structure to hold 4-component vector data
struct svrVector4
{
    float x,y,z,w;
};

//! \brief Simple structure to hold quaternion data
struct svrQuaternion
{
    float x, y, z, w;
};

//! \brief Simple structure to hold 4x4 matrix data
struct svrMatrix4
{
    float M[4][4];
};

//! \brief Enum used to indicate which eye is being used
enum svrWhichEye
{
    kLeftEye = 0,
    kRightEye,
    kNumEyes
};

//! \brief Enum used to indicate which eye to apply a render layer
enum svrEyeMask
{
    kEyeMaskLeft = 0x00000001,
    kEyeMaskRight = 0x00000002,
    kEyeMaskBoth = 0x00000003
};

//! \brief Structure containing the position and orientation of the head
struct svrHeadPose
{
    svrQuaternion   rotation;
    svrVector3      position;
};

//! \brief Enum used to indicate valid components of an svrHeadPose
enum svrTrackingMode
{
    kTrackingRotation = (1 << 0),
    kTrackingPosition = (1 << 1)
};

//! \brief Structure containing the full set of pose data
struct svrHeadPoseState
{
    svrHeadPose         pose;                   //!< Head pose
    int                 poseStatus;             //!< Bit field (svrTrackingMode) indicating head pose status
    uint64_t            poseTimeStampNs;        //!< Time stamp in which the head pose was generated (nanoseconds)
    svrVector3          angularVelocity;        //!< Angular velocity
    svrVector3          linearVelocity;         //!< Linear velocity
    svrVector3          angularAcceleration;    //!< Angular acceleration
    svrVector3          linearAccelearation;    //!< Linear acceleration
    float               predictedTimeMs;        //!< Prediction time used to generate the head pose (milliseconds)
};

//! \brief Enum used for indicating the CPU/GPU performance levels
//! \sa svrBeginVr, svrSetPerformanceLevels
enum svrPerfLevel
{
    kPerfSystem     = 0,            //!< System defined performance level (default)
    kPerfMinimum    = 1,            //!< Minimum performance level 
    kPerfMedium     = 2,            //!< Medium performance level 
    kPerfMaximum    = 3,            //!< Maximum performance level
    kNumPerfLevels
};

//! \brief Structure containing parameters needed to enable VR mode
//! \sa svrBeginVr
struct svrBeginParams
{
    int             mainThreadId;           //!< Thread Id of the primary render thread
    svrPerfLevel    cpuPerfLevel;           //!< Desired CPU performance level
    svrPerfLevel    gpuPerfLevel;           //!< Desired GPU performance level
    ANativeWindow*  nativeWindow;           //!< Pointer to the Android native window
    bool            isProtectedContent;     //!< Protected content or not
};

//! \brief Options which can be set when submitting a frame to modify the behavior of asynchronous time warp
//! \sa svrSubmitFrame
enum svrFrameOption
{
    kDisableDistortionCorrection    = ( 1 << 0 ),   //!< Disables the lens distortion correction (useful for debugging)
    kDisableReprojection            = ( 1 << 1 ),   //!< Disables re-projection
    kEnableMotionToPhoton           = ( 1 << 2 ),   //!< Enables motion to photon testing 
    kDisableChromaticCorrection     = ( 1 << 3 )   //!< Disables the lens chromatic aberration correction (performance optimization)
};

//! \brief Enum used to indicate the layout of the eye buffers being submitted to asynchronous time warp
//! \sa svrSubmitFrame
enum svrEyeBufferType
{
    kEyeBufferMono,                 //!< Single eye buffer which will be duplicated in the left and right eyes
    kEyeBufferStereoSeparate,       //!< Separate eye buffers for the left and right eyes
    kEyeBufferStereoSingle,         //!< Single double-wide eye buffer containing both the left and right eyes
    kEyeBufferArray,                //!< Single array where the 1st and 2nd slices contain the left and right eye buffers
};

//! \brief Enum used to indicate the type of texture passed in as an overlay buffer
//! \sa svrSubmitFrame
enum svrOverlayType
{
    kOverlayMono = 0,               //!< Same full screen image used on both eyes
    kOverlayStereo,                 //!< Separate fullscreen image overlay for the left and right eyes
    kOverlayLayers,                 //!< Each layer is rendered using the coordinates specified. Same render on each eye
};

//! \brief Enum used to indicate the type of texture passed in as a render layer
//! \sa svrSubmitFrame
enum svrTextureType
{
    kTypeTexture = 0,               //!< Standard texture
    kTypeImage,                     //!< EGL Image texture
    kTypeVulkan,                    //!< Vulkan texture
    kTypeEquiRectTexture,           //!< Equirectangular texture
    kTypeEquiRectImage,             //!< Equirectangular Image texture
};

//! \brief Information about texture if type is kTypeVulkan
//! \sa svrSubmitFrame
struct svrVulkanTexInfo
{
    uint32_t            memSize;
    uint32_t            width;
    uint32_t            height;
    uint32_t            numMips;
    uint32_t            bytesPerPixel;
    uint32_t            renderSemaphore;
};


//! \brief Enum used to indicate the type of warp/composition that should be used for a frame
enum svrWarpType
{
    kSimple                         //!< Basic single layer (world) warp 
};

//! \brief Enumeration of possible warp mesh types
//! \sa svrDeviceInfo
enum svrWarpMeshType
{
    kMeshTypeColumsLtoR = 0,    // Columns Left to Right
    kMeshTypeColumsRtoL,        // Columns Right to Left
    kMeshTypeRowsTtoB,          // Rows Top to Bottom
    kMeshTypeRowsBtoT,          // Rows Bottom to Top
};

//! \brief Enumeration of possible warp meshes
//! \sa svrSetWarpMesh, svrWarpMeshType
enum svrWarpMeshEnum
{
    kMeshEnumLeft = 0,  // Column mesh for left half of screen
    kMeshEnumRight,     // Column mesh for right half of screen
    kMeshEnumUL,        // Row mesh for Upper Left part of the screen
    kMeshEnumUR,        // Row mesh for Upper Right part of the screen
    kMeshEnumLL,        // Row mesh for Lower Left part of the screen
    kMeshEnumLR,        // Row mesh for Lower Right part of the screen
    kWarpMeshCount
};

//! \brief Render layer screen position and UV coordinates
//! \sa svrSubmitFrame
struct svrLayoutCoords
{
    float               LowerLeftPos[4];                        //!< 0 = X-Position; 1 = Y-Position; 2 = Z-Position; 3 = Padding
    float               LowerRightPos[4];                       //!< 0 = X-Position; 1 = Y-Position; 2 = Z-Position; 3 = Padding
    float               UpperLeftPos[4];                        //!< 0 = X-Position; 1 = Y-Position; 2 = Z-Position; 3 = Padding
    float               UpperRightPos[4];                       //!< 0 = X-Position; 1 = Y-Position; 2 = Z-Position; 3 = Padding

    float               LowerUVs[4];                            //!< [0,1] = Lower Left UV values; [2,3] = Lower Right UV values
    float               UpperUVs[4];                            //!< [0,1] = Upper Left UV values; [2,3] = Upper Right UV values

    float               TransformMatrix[16];                    //!< Column major uv transform matrix data. Applies to video textures (see SurfaceTexture::getTransformMatrix())
};

//! \brief Description of render layers
//! \sa svrSubmitFrame
struct svrRenderLayer
{
    int                 imageHandle;        //!< Handle to the texture/image to be rendered
    svrTextureType      imageType;          //!< Type of texture: Standard Texture or EGL Image
    svrLayoutCoords     imageCoords;        //!< Layout of this layer on the screen
    svrEyeMask          eyeMask;            //!< Determines which eye[s] receive this render layer
    svrVulkanTexInfo    vulkanInfo;         //!< Information about the data if it is a Vulkan texture
};


//! \brief Per-frame data needed for time warp, distortion/aberration correction
//! \sa svrSubmitFrame
struct svrFrameParams
{
    int                 frameIndex;                             //!< Frame Index
    int                 minVsyncs;                              //!< Minimum number of vysnc events before displaying the frame (1=display refresh, 2=half refresh, etc...)

    svrEyeBufferType    eyeBufferType;                          //!< Layout for the supplied eye buffer(s)

    svrRenderLayer      eyeLayers[SVR_MAX_EYE_LAYERS];          //!< Description of each render layer (will be reprojected)
    svrRenderLayer      overlayLayers[SVR_MAX_OVERLAY_LAYERS];  //!< Description of each overlay layer (will not be reprojected)

    unsigned int        frameOptions;                           //!< Options for adjusting the frame warp behavior (bitfield of svrFrameOption)
    svrHeadPoseState    headPoseState;                          //!< Head pose state used to generate the frame  
    svrWarpType         warpType;                               //!< Type of warp to be used on the frame
    float               fieldOfView;                            //!< Field of view used to generate this frame (larger than device fov to provide timewarp margin)
};

//! \brief Initialization parameters that are constant over the life-cycle of the application
//! \sa svrInitialize
struct svrInitParams
{
    JavaVM*         javaVm;                 //!< Java Virtual Machine pointer
    JNIEnv*         javaEnv;                //!< Java Environment
    jobject         javaActivityObject;     //!< Reference to the Android activity
};

//! \brief View Frustum.  These values are based on physical device properties, except the far plane is arbitrary
//! \sa svrSubmitFrame
struct svrViewFrustum
{
    float               left;           //!< Left Plane of Frustum
    float               right;          //!< Right Plane of Frustum
    float               top;            //!< Top Plane of Frustum
    float               bottom;         //!< Bottom Plane of Frustum

    float               near;           //!< Near Plane of Frustum
    float               far;            //!< Far Plane of Frustum (Arbitrary)
};

//! \brief Basic device information to allow the client code to optimally setup their simulation and rendering pipelines
struct svrDeviceInfo
{
    int             displayWidthPixels;         //!< Physical width of the display (pixels)
    int             displayHeightPixels;        //!< Physical height of the display (pixels)
    float           displayRefreshRateHz;       //!< Refresh rate of the display
    int             displayOrientation;         //!< Display orientation (degrees at initialization - 0,90,180,270)
    int             targetEyeWidthPixels;       //!< Recommended eye buffer width (pixels)
    int             targetEyeHeightPixels;      //!< Recommended eye buffer height (pixels)
    float           targetFovXRad;              //!< Recommended horizontal FOV
    float           targetFovYRad;              //!< Recommended vertical FOV
    svrViewFrustum  leftEyeFrustum;             //!< Recommended Frustum information for left eye
    svrViewFrustum  rightEyeFrustum;            //!< Recommended Frustum information for right eye
    int             deviceOSVersion;            //!< Android OS Version of the device
    svrWarpMeshType warpMeshType;               //!< Type of mesh used to render eye buffer
};

enum svrThermalLevel
{
    kSafe,
    kLevel1,
    kLevel2,
    kLevel3,
    kCritical,
    kNumThermalLevels
};

enum svrThermalZone
{
    kCpu,
    kGpu,
    kSkin,
    kNumThermalZones
};

struct svrEventData_Thermal
{
    svrThermalZone  zone;               //!< Thermal zone
    svrThermalLevel level;              //!< Indication of the current zone thermal level
};

typedef union 
{
    svrEventData_Thermal    thermal;
    unsigned int            data[2];
} svrEventData;

struct svrEvent
{
    svrEventType    eventType;              //!< Type of event
    unsigned int    deviceId;               //!< An identifier for the device that generated the event (0 == HMD)
    float           eventTimeStamp;         //!< Time stamp for the event in seconds since the last svrBeginVr call
    svrEventData    eventData;              //!< Event specific data payload
};

//! \brief Events to use in svrControllerSendMessage
enum svrControllerMessageType
{
    kControllerMessageRecenter = 0,
    kControllerMessageVibration = 1
};

//! \brief Query Values
enum svrControllerQueryType
{
    kControllerQueryBatteryRemaining = 0,
    kControllerQueryControllerCaps = 1
};

//! Controller Connection state
enum svrControllerConnectionState {
    kNotInitialized = 0,
    kDisconnected = 1,
    kConnected = 2,
    kConnecting = 3,
    kError = 4
};

//! Controller Touch button enumerations
namespace svrControllerTouch {
  enum {
    None                = 0x00000000,
    One                 = 0x00000001,
    Two                 = 0x00000002,
    Three               = 0x00000004,
    Four                = 0x00000008,
    PrimaryThumbstick   = 0x00000010,
    SecondaryThumstick  = 0x00000020,
    Any                 = ~None
  };
}

//! Controller Trigger enumerations
namespace svrControllerAxis1D {
enum {
  PrimaryIndexTrigger   = 0x00000000,
  SecondaryIndexTrigger = 0x00000001,
  PrimaryHandTrigger    = 0x00000002,
  SecondaryHandTrigger  = 0x00000003
};
}

//! Controller Joystick enumerations
namespace svrControllerAxis2D {
enum {
  PrimaryThumbstick     = 0x00000000,
  SecondaryThumbstick   = 0x00000001
};
}

//! Controller Button enumerations
namespace svrControllerButton {
enum {
  None                    = 0x00000000,
  One                     = 0x00000001,
  Two                     = 0x00000002,
  Three                   = 0x00000004,
  Four                    = 0x00000008,
  DpadUp                  = 0x00000010,
  DpadDown                = 0x00000020,
  DpadLeft                = 0x00000040,
  DpadRight               = 0x00000080,
  Start                   = 0x00000100,
  Back                    = 0x00000200,
  PrimaryShoulder         = 0x00001000,
  PrimaryIndexTrigger     = 0x00002000,
  PrimaryHandTrigger      = 0x00004000,
  PrimaryThumbstick       = 0x00008000,
  PrimaryThumbstickUp     = 0x00010000,
  PrimaryThumbstickDown   = 0x00020000,
  PrimaryThumbstickLeft   = 0x00040000,
  PrimaryThumbstickRight  = 0x00080000,
  SecondaryShoulder       = 0x00100000,
  SecondaryIndexTrigger   = 0x00200000,
  SecondaryHandTrigger    = 0x00400000,
  SecondaryThumbstick     = 0x00800000,
  SecondaryThumbstickUp   = 0x01000000,
  SecondaryThumbstickDown = 0x02000000,
  SecondaryThumbstickLeft = 0x04000000,
  SecondaryThumbstickRight = 0x08000000,
  Up                      = 0x10000000,
  Down                    = 0x20000000,
  Left                    = 0x40000000,
  Right                   = 0x80000000,
  Any                     = ~None
};
}

// Current state of the controller
struct svrControllerState {
    //! Orientation
    svrQuaternion   rotation;

    //! Position
    svrVector3      position;
    
    //! Gyro
    svrVector3      gyroscope;
    
    //! Accelerometer
    svrVector3      accelerometer;
    
    //! timestamp
    uint64_t        timestamp;

    //! All digital button states as bitflags
    uint32_t        buttonState;   //!< all digital as bit flags
    
    //! Touchpads, Joysticks
    svrVector2      analog2D[4];       //!< analog 2D's

    //! Triggers
    float           analog1D[8];       //!< analog 1D's

    //! Whether the touchpad area is being touched.
    uint32_t        isTouching;
     
    //! Controller Connection state
    svrControllerConnectionState connectionState;
};

// Caps of the Controller.
struct svrControllerCaps {
    //! Device Manufacturer
    char deviceManufacturer[64];
    
    //! Device Identifier from the module
    char deviceIdentifier[64];
    
    //! Controller Capabilities
    uint32_t caps; //0 bit = Provides Rotation; 1 bit = Position;

    //! Enabled Buttons Bitfield
    uint32_t activeButtons;
    
    //! Active 2D Analogs Bitfield
    uint32_t active2DAnalogs;
    
    //! Active 1D Analogs Bitfield
    uint32_t active1DAnalogs;
    
    //! Active Touch Buttons
    uint32_t activeTouchButtons;
};

#ifndef SVRP_EXPORT
    #define SVRP_EXPORT
#endif

#ifdef __cplusplus 
extern "C" {
#endif

//! \brief Returns the VR SDK version string
SVRP_EXPORT const char* svrGetVersion();

//! \brief Returns the VR service version string
SVRP_EXPORT SvrResult svrGetVrServiceVersion(char *pRetBuffer, int bufferSize);

//! \brief Returns the VR client version string
SVRP_EXPORT SvrResult svrGetVrClientVersion(char *pRetBuffer, int bufferSize);

//! \brief Initializes VR components 
//! \param pInitParams svrInitParams structure
SVRP_EXPORT SvrResult svrInitialize(const svrInitParams* pInitParams);

//! \brief Releases VR components
SVRP_EXPORT SvrResult svrShutdown();

//! \brief Queries for device specific information
//! \return svrDeviceInfo structure containing device specific information (resolution, fov, etc..)
SVRP_EXPORT svrDeviceInfo   svrGetDeviceInfo();

//! \brief Requests specific brackets of CPU/GPU performance
//! \param cpuPerfLevel Requested performance level for CPU
//! \param gpuPerfLevel Requested performance level for GPU
SVRP_EXPORT SvrResult svrSetPerformanceLevels(svrPerfLevel cpuPerfLevel, svrPerfLevel gpuPerfLevel);

//! \brief Enables VR services
//! \param pBeginParams svrBeginParams structure
SVRP_EXPORT SvrResult svrBeginVr(const svrBeginParams* pBeginParams);

//! \brief Disables VR services
SVRP_EXPORT SvrResult svrEndVr();

//! \brief Calculates a predicted time when the current frame would be displayed
//! \return Predicted display time for the current frame in milliseconds
SVRP_EXPORT float svrGetPredictedDisplayTime();

//! \brief Calculates a predicted head pose
//! \param predictedTimeMs Time ahead of the current time in ms to predict a head pose for
//! \return The predicted head pose and relevant pose state information
SVRP_EXPORT svrHeadPoseState svrGetPredictedHeadPose( float predictedTimeMs );

//! \brief Retrieves a historic head pose
//! \param predictedTimeMs Time in ns to retrieve a head pose for
//! \return The head pose and relevant pose state information
SVRP_EXPORT svrHeadPoseState svrGetHistoricHeadPose(int64_t timestampNs);

//! \brief Recenters the head position and orientation at the current values
SVRP_EXPORT SvrResult svrRecenterPose();

//! \brief Recenters the head position at the current position
SVRP_EXPORT SvrResult svrRecenterPosition();

//! \brief Recenters the head orientation (Yaw only) at the current value
SVRP_EXPORT SvrResult svrRecenterOrientation(bool yawOnly=true);

//! \brief Returns the supported tracking types
//! \return Bitfield of svrTrackingType values indicating the supported tracking modes
SVRP_EXPORT unsigned int svrGetSupportedTrackingModes();

//! \brief Sets the current head tracking mode
//! \param trackingModes Bitfield of svrTrackingType values indicating the tracking modes to enable
SVRP_EXPORT SvrResult svrSetTrackingMode(unsigned int trackingModes);

//! \brief Returns the current head tracking mode
//! \return trackingMode Bitfield of svrTrackingType values indicating the tracking modes enabled
SVRP_EXPORT unsigned int svrGetTrackingMode();

//! \brief Polls for an available event.  If event(s) are present pEvent will be filled with the event details
//! \return true if an event was present, false if not
SVRP_EXPORT bool svrPollEvent(svrEvent *pEvent);

//! \brief Called after eye buffer is bound but before game rendering starts
//! \param eyeBufferType Type of bound render buffer
//! \param whichEye Which eye is being rendered
SVRP_EXPORT SvrResult svrBeginEye(svrEyeBufferType eyeBufferType, svrWhichEye whichEye);

//! \brief Called after eye buffer is rendered but before frame is submitted
//! \param eyeBufferType Type of bound render buffer
//! \param whichEye Which eye is being rendered
SVRP_EXPORT SvrResult svrEndEye(svrEyeBufferType eyeBufferType, svrWhichEye whichEye);

//! \brief Submits a frame to asynchronous time warp
//! \param pFrameParams svrFrameParams structure
SVRP_EXPORT SvrResult svrSubmitFrame(const svrFrameParams* pFrameParams);

//! \brief Starts Tracking 
//! \param controllerDesc Controller Description
//! \return handle for the controller
SVRP_EXPORT int svrControllerStartTracking(const char* controllerDesc);

//! \brief Stops tracking the controller
//! \param controllerHandle handle for the controller
SVRP_EXPORT void svrControllerStopTracking(int controllerHandle);

//! \brief Get the current state of the controller
//! \param controllerHandle handle for the controller
SVRP_EXPORT svrControllerState svrControllerGetState(int controllerHandle);

//! \brief Send a message to the controller
//! \param controllerHandle handle for the controller
//! \param what type of event
//! \param arg1 argument for the event
//! \param arg2 argument for the event
SVRP_EXPORT void svrControllerSendMessage(int controllerHandle, int what, int arg1, int arg2);

//! \brief make a query to the controller
//! \param controllerHandle handle for the controller
//! \param what info
//! \param memory memory to fill in
//! \param memorySize size of the memory to fill in
//! \return number of bytes written into the memory
SVRP_EXPORT int svrControllerQuery(int controllerHandle, int what, void* memory, unsigned int memorySize);

//! \brief Replaces the current Time Warp mesh the supplied mesh
//! \param whichMesh Which mesh is to be replaced
//! \param pVertexData 15 float values per vertex: Position = 3; R|G|B UV values 4 each (Fourth value is Vignette multiplier [0,1]). NULL reverts the override.
//! \param vertexSize Size of pVertexData
//! \param nVertices Number of vertices contained in pVertexData
//! \param pIndices Pointer to indice data. Expected to be used with GL_TRIANGLES
//! \param nIndices Number of indices contained in pIndices. Expected to be used with GL_TRIANGLES
//! \return SVR_ERROR_NONE for success; SVR_ERROR_UNSUPPORTED if vertexSize is incorrect; SVR_ERROR_UNKNOWN for any other error
//! \sa svrWarpMeshType, svrWarpMeshEnum
SVRP_EXPORT SvrResult svrSetWarpMesh(svrWarpMeshEnum whichMesh, void *pVertexData, int vertexSize, int nVertices, unsigned int* pIndices, int nIndices);

#ifdef __cplusplus 
}
#endif

#endif //_SVR_API_H_
