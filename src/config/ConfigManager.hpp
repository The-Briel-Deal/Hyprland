#pragma once

#define CONFIG_MANAGER_H

#include <map>
#include "../debug/Log.hpp"
#include <unordered_map>
#include "../defines.hpp"
#include <vector>
#include <deque>
#include <algorithm>
#include <regex>
#include <optional>
#include <functional>
#include <xf86drmMode.h>
#include "../Window.hpp"
#include "../helpers/WLClasses.hpp"
#include "../helpers/Monitor.hpp"
#include "../helpers/VarList.hpp"

#include "defaultConfig.hpp"
#include "ConfigDataValues.hpp"

#include <hyprlang.hpp>

#define INITANIMCFG(name)           animationConfig[name] = {}
#define CREATEANIMCFG(name, parent) animationConfig[name] = {false, "", "", 0.f, -1, &animationConfig["global"], &animationConfig[parent]}

#define HANDLE void*

struct SWorkspaceRule {
    std::string                        monitor         = "";
    std::string                        workspaceString = "";
    std::string                        workspaceName   = "";
    int                                workspaceId     = -1;
    bool                               isDefault       = false;
    bool                               isPersistent    = false;
    std::optional<int64_t>             gapsIn;
    std::optional<int64_t>             gapsOut;
    std::optional<int64_t>             borderSize;
    std::optional<int>                 border;
    std::optional<int>                 rounding;
    std::optional<int>                 decorate;
    std::optional<int>                 shadow;
    std::optional<std::string>         onCreatedEmptyRunCmd;
    std::map<std::string, std::string> layoutopts;
};

struct SMonitorAdditionalReservedArea {
    int top    = 0;
    int bottom = 0;
    int left   = 0;
    int right  = 0;
};

struct SAnimationPropertyConfig {
    bool                      overridden = true;

    std::string               internalBezier  = "";
    std::string               internalStyle   = "";
    float                     internalSpeed   = 0.f;
    int                       internalEnabled = -1;

    SAnimationPropertyConfig* pValues          = nullptr;
    SAnimationPropertyConfig* pParentAnimation = nullptr;
};

struct SPluginKeyword {
    HANDLE                                                      handle = 0;
    std::string                                                 name   = "";
    std::function<void(const std::string&, const std::string&)> fn;
};

struct SExecRequestedRule {
    std::string szRule = "";
    uint64_t    iPid   = 0;
};

class CConfigManager {
  public:
    CConfigManager();

    void                                                            tick();
    void                                                            init();

    int                                                             getDeviceInt(const std::string&, const std::string&, const std::string& fallback = "");
    float                                                           getDeviceFloat(const std::string&, const std::string&, const std::string& fallback = "");
    Vector2D                                                        getDeviceVec(const std::string&, const std::string&, const std::string& fallback = "");
    std::string                                                     getDeviceString(const std::string&, const std::string&, const std::string& fallback = "");
    bool                                                            deviceConfigExists(const std::string&);
    Hyprlang::CConfigValue*                                         getConfigValueSafeDevice(const std::string& dev, const std::string& val, const std::string& fallback);
    bool                                                            shouldBlurLS(const std::string&);

    void* const*                                                    getConfigValuePtr(const std::string&);
    Hyprlang::CConfigValue*                                         getHyprlangConfigValuePtr(const std::string&);
    static std::string                                              getConfigDir();
    static std::string                                              getMainConfigPath();

    SMonitorRule                                                    getMonitorRuleFor(const std::string&, const std::string& displayName = "");
    SWorkspaceRule                                                  getWorkspaceRuleFor(CWorkspace*);
    std::string                                                     getDefaultWorkspaceFor(const std::string&);

    CMonitor*                                                       getBoundMonitorForWS(const std::string&);
    std::string                                                     getBoundMonitorStringForWS(const std::string&);
    const std::deque<SWorkspaceRule>&                               getAllWorkspaceRules();

    std::vector<SWindowRule>                                        getMatchingRules(CWindow*);
    std::vector<SLayerRule>                                         getMatchingRules(SLayerSurface*);

    std::unordered_map<std::string, SMonitorAdditionalReservedArea> m_mAdditionalReservedAreas;

    std::unordered_map<std::string, SAnimationPropertyConfig>       getAnimationConfig();

    void                                                            addPluginConfigVar(HANDLE handle, const std::string& name, const Hyprlang::CConfigValue& value);
    void addPluginKeyword(HANDLE handle, const std::string& name, std::function<void(const std::string& cmd, const std::string& val)> fun);
    void removePluginConfig(HANDLE handle);

    // no-op when done.
    void                      dispatchExecOnce();

    void                      performMonitorReload();
    bool                      m_bWantsMonitorReload = false;
    bool                      m_bForceReload        = false;
    bool                      m_bNoMonitorReload    = false;
    void                      ensureMonitorStatus();
    void                      ensureVRR(CMonitor* pMonitor = nullptr);

    std::string               parseKeyword(const std::string&, const std::string&);

    void                      addParseError(const std::string&);

    SAnimationPropertyConfig* getAnimationPropertyConfig(const std::string&);

    void                      addExecRule(const SExecRequestedRule&);

    void                      handlePluginLoads();

    // keywords
    void        handleRawExec(const std::string&, const std::string&);
    void        handleMonitor(const std::string&, const std::string&);
    void        handleBind(const std::string&, const std::string&);
    void        handleUnbind(const std::string&, const std::string&);
    void        handleWindowRule(const std::string&, const std::string&);
    void        handleLayerRule(const std::string&, const std::string&);
    void        handleWindowRuleV2(const std::string&, const std::string&);
    void        handleWorkspaceRules(const std::string&, const std::string&);
    void        handleBezier(const std::string&, const std::string&);
    void        handleAnimation(const std::string&, const std::string&);
    void        handleSource(const std::string&, const std::string&);
    void        handleSubmap(const std::string&, const std::string&);
    void        handleBlurLS(const std::string&, const std::string&);
    void        handleBindWS(const std::string&, const std::string&);
    void        handleEnv(const std::string&, const std::string&);
    void        handlePlugin(const std::string&, const std::string&);

    std::string configCurrentPath;

  private:
    std::unique_ptr<Hyprlang::CConfig>                        m_pConfig;

    std::deque<std::string>                                   configPaths;       // stores all the config paths
    std::unordered_map<std::string, time_t>                   configModifyTimes; // stores modify times

    std::unordered_map<std::string, SAnimationPropertyConfig> animationConfig; // stores all the animations with their set values

    std::string                                               parseError = ""; // For storing a parse error to display later

    std::string                                               m_szCurrentSubmap = ""; // For storing the current keybind submap

    std::vector<SExecRequestedRule>                           execRequestedRules; // rules requested with exec, e.g. [workspace 2] kitty

    std::vector<std::string>                                  m_vDeclaredPlugins;
    std::vector<SPluginKeyword>                               pluginKeywords;

    bool                                                      isFirstLaunch = true; // For exec-once

    std::deque<SMonitorRule>                                  m_dMonitorRules;
    std::deque<SWorkspaceRule>                                m_dWorkspaceRules;
    std::deque<SWindowRule>                                   m_dWindowRules;
    std::deque<SLayerRule>                                    m_dLayerRules;
    std::deque<std::string>                                   m_dBlurLSNamespaces;

    bool                                                      firstExecDispatched     = false;
    bool                                                      m_bManualCrashInitiated = false;
    std::deque<std::string>                                   firstExecRequests;

    std::vector<std::pair<std::string, std::string>>          m_vFailedPluginConfigValues; // for plugin values of unloaded plugins

    // internal methods
    void setAnimForChildren(SAnimationPropertyConfig* const);
    void updateBlurredLS(const std::string&, const bool);
    void setDefaultAnimationVars();
    void reload();
};

inline std::unique_ptr<CConfigManager> g_pConfigManager;
