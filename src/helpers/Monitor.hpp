#pragma once

#include "../defines.hpp"
#include <deque>
#include "WLClasses.hpp"
#include <vector>
#include <array>
#include <memory>
#include <xf86drmMode.h>
#include "Timer.hpp"
#include "Region.hpp"
#include <optional>

// Enum for the different types of auto directions, e.g. auto-left, auto-up.
enum class AutoDirs {
    auto_up,
    auto_down,
    auto_left,
    auto_right
};

struct SMonitorRule {
    AutoDirs            autoDir;
    std::string         name        = "";
    Vector2D            resolution  = Vector2D(1280, 720);
    Vector2D            offset      = Vector2D(0, 0);
    float               scale       = 1;
    float               refreshRate = 60;
    bool                disabled    = false;
    wl_output_transform transform   = WL_OUTPUT_TRANSFORM_NORMAL;
    std::string         mirrorOf    = "";
    bool                enable10bit = false;
    drmModeModeInfo     drmMode     = {};
    std::optional<int>  vrr;
};

class CMonitor;

// Class for wrapping the wlr state
class CMonitorState {
  public:
    CMonitorState(CMonitor* owner);
    ~CMonitorState();

    wlr_output_state* wlr();
    void              clear();
    // commit() will also clear()
    bool commit();
    bool test();

  private:
    wlr_output_state m_state = {0};
    CMonitor*        m_pOwner;
};

class CMonitor {
  public:
    CMonitor();
    ~CMonitor();

    Vector2D        vecPosition         = Vector2D(-1, -1); // means unset
    Vector2D        vecXWaylandPosition = Vector2D(-1, -1); // means unset
    Vector2D        vecSize             = Vector2D(0, 0);
    Vector2D        vecPixelSize        = Vector2D(0, 0);
    Vector2D        vecTransformedSize  = Vector2D(0, 0);

    bool            primary = false;

    uint64_t        ID                     = -1;
    PHLWORKSPACE    activeWorkspace        = nullptr;
    PHLWORKSPACE    activeSpecialWorkspace = nullptr;
    float           setScale               = 1; // scale set by cfg
    float           scale                  = 1; // real scale

    std::string     szName             = "";
    std::string     szDescription      = "";
    std::string     szShortDescription = "";

    Vector2D        vecReservedTopLeft     = Vector2D(0, 0);
    Vector2D        vecReservedBottomRight = Vector2D(0, 0);

    drmModeModeInfo customDrmMode = {};

    CMonitorState   state;

    // WLR stuff
    wlr_damage_ring         damage;
    wlr_output*             output          = nullptr;
    float                   refreshRate     = 60;
    int                     framesToSkip    = 0;
    int                     forceFullFrames = 0;
    bool                    noFrameSchedule = false;
    bool                    scheduledRecalc = false;
    wl_output_transform     transform       = WL_OUTPUT_TRANSFORM_NORMAL;
    bool                    gammaChanged    = false;
    float                   xwaylandScale   = 1.f;
    std::array<float, 9>    projMatrix      = {0};
    std::optional<Vector2D> forceSize;

    bool                    dpmsStatus       = true;
    bool                    vrrActive        = false; // this can be TRUE even if VRR is not active in the case that this display does not support it.
    bool                    enabled10bit     = false; // as above, this can be TRUE even if 10 bit failed.
    bool                    createdByUser    = false;
    uint32_t                drmFormat        = DRM_FORMAT_INVALID;
    bool                    isUnsafeFallback = false;

    bool                    pendingFrame    = false; // if we schedule a frame during rendering, reschedule it after
    bool                    renderingActive = false;

    wl_event_source*        renderTimer  = nullptr; // for RAT
    bool                    RATScheduled = false;
    CTimer                  lastPresentationTimer;

    SMonitorRule            activeMonitorRule;

    // mirroring
    CMonitor*              pMirrorOf = nullptr;
    std::vector<CMonitor*> mirrors;

    // for tearing
    CWindow* solitaryClient = nullptr;

    struct {
        bool canTear         = false;
        bool nextRenderTorn  = false;
        bool activelyTearing = false;

        bool busy                    = false;
        bool frameScheduledWhileBusy = false;
    } tearingState;

    std::array<std::vector<std::unique_ptr<SLayerSurface>>, 4> m_aLayerSurfaceLayers;

    DYNLISTENER(monitorFrame);
    DYNLISTENER(monitorDestroy);
    DYNLISTENER(monitorStateRequest);
    DYNLISTENER(monitorDamage);
    DYNLISTENER(monitorNeedsFrame);
    DYNLISTENER(monitorCommit);
    DYNLISTENER(monitorBind);

    // methods
    void     onConnect(bool noRule);
    void     onDisconnect(bool destroy = false);
    void     addDamage(const pixman_region32_t* rg);
    void     addDamage(const CRegion* rg);
    void     addDamage(const CBox* box);
    void     setMirror(const std::string&);
    bool     isMirror();
    bool     matchesStaticSelector(const std::string& selector) const;
    float    getDefaultScale();
    void     changeWorkspace(const PHLWORKSPACE& pWorkspace, bool internal = false, bool noMouseMove = false, bool noFocus = false);
    void     changeWorkspace(const int& id, bool internal = false, bool noMouseMove = false, bool noFocus = false);
    void     setSpecialWorkspace(const PHLWORKSPACE& pWorkspace);
    void     setSpecialWorkspace(const int& id);
    void     moveTo(const Vector2D& pos);
    Vector2D middle();
    void     updateMatrix();
    int64_t  activeWorkspaceID();
    int64_t  activeSpecialWorkspaceID();

    bool     m_bEnabled             = false;
    bool     m_bRenderingInitPassed = false;

    // For the list lookup

    bool operator==(const CMonitor& rhs) {
        return vecPosition == rhs.vecPosition && vecSize == rhs.vecSize && szName == rhs.szName;
    }

  private:
    void setupDefaultWS(const SMonitorRule&);
    int  findAvailableDefaultWS();
};
