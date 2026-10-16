

#ifdef __APPLE__
  #include <GLUT/glut.h>
#elif defined(_WIN32)
  #include <windows.h>
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
  #include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define PI 3.14159265358979323846f
#define TWO_PI (2.0f * PI)
#define HALF_PI (0.5f * PI)
#define DEG2RAD(d) ((d) * PI / 180.0f)
#define RAD2DEG(r) ((r) * 180.0f / PI)
#define CLAMP(v,a,b) ((v)<(a)?(a):((v)>(b)?(b):(v)))
#define LERP(a,b,t) ((a)+((b)-(a))*(t))
#define SQ(x) ((x)*(x))
#define DIST2(ax,ay,az,bx,by,bz) std::sqrt(SQ((ax)-(bx))+SQ((ay)-(by))+SQ((az)-(bz)))

static int WIN_W = 1600;
static int WIN_H = 900;

static const int NP = 9;
static const int TRAIL_LEN = 220;
static const int N_GALAXY_ARMS = 8;
static const int STARS_PER_ARM = 4200;
static const int N_CORE_STARS = 4200;
static const int N_BAR_STARS = 1800;
static const int N_BACKGROUND_STARS = 10000;
static const int N_ARM_CLUSTERS = 560;
static const int STARS_PER_CLUSTER = 12;
static const int N_DUST_LANES = 3600;
static const int N_HALO_STARS = 1100;
static const int N_STREAM_STARS = 700;
static const int NSTARS = N_CORE_STARS + N_BAR_STARS + N_GALAXY_ARMS * STARS_PER_ARM +
                          N_BACKGROUND_STARS + N_ARM_CLUSTERS * STARS_PER_CLUSTER +
                          N_DUST_LANES + N_HALO_STARS + N_STREAM_STARS;
static const int NNEB = 120;
static const int N_SFR = 48;
static const int NAST = 420;
static const int COMET_PARTS = 180;
static const int N_METEORS = 28;
static const int N_LENS_RAYS = 14;
static const int N_HELP_LINES = 26;
static const int N_DEEP_GALAXIES = 18;
static const int N_QUASARS = 10;
static const int N_NEARBY_SYSTEMS = 7;
static const int N_OUTER_BODIES = 5;

static const int EIDX = 2;
static const int JIDX = 4;
static const int SIDX = 5;
static const int UIDX = 6;
static const int NIDX = 7;
static const int PIDX = 8;

static const char* ARM_NAMES[N_GALAXY_ARMS] = {
    "Perseus Arm",
    "Outer Arm",
    "Cygnus Arm",
    "Orion Spur (Local Arm)",
    "Carina-Sagittarius Arm",
    "Scutum-Centaurus Arm",
    "Norma Arm",
    "Swan Arm"
};

static const float ARM_COLORS[N_GALAXY_ARMS][3] = {
    {0.65f,0.85f,1.00f},
    {0.55f,0.70f,1.00f},
    {0.75f,0.90f,1.00f},
    {1.00f,1.00f,0.35f},
    {1.00f,0.55f,0.30f},
    {0.40f,1.00f,0.65f},
    {0.90f,0.40f,1.00f},
    {0.60f,0.80f,1.00f}
};

enum CameraMode { CAM_FREE=0, CAM_FOLLOW=1, CAM_ORBIT=2, CAM_CINEMATIC=3 };
enum ViewMode { VIEW_SOLAR=0, VIEW_GALAXY=1 };

struct Camera
{
    float dist, yaw, pitch;
    float targetX, targetY, targetZ;
    float smoothX, smoothY, smoothZ;
    CameraMode mode;
    int focusPlanet;
    bool transitioning;
    float transT;
    float fromDist, toDist;
    float fromYaw, toYaw;
    float fromPitch, toPitch;
    float fromTX, fromTY, fromTZ;
    float toTX, toTY, toTZ;
    float orbitAngle;
    float cinT, cinDuration;
    float cinFromX, cinFromY, cinFromZ;
    float cinToX, cinToY, cinToZ;
};

struct Planet
{
    const char* name;
    const char* fact;
    const char* detail;
    const char* composition;
    float orbitRadius, size, orbitSpeed, spinSpeed;
    float orbitAngle, spinAngle, tilt, eccentricity;
    float r, g, b, spec;
    float realDiameter, realDistance, realOrbitalPeriod;
    float realDayLength, gravity, temperature;
    int moons;
    float wx, wy, wz;
    float tx[TRAIL_LEN], ty[TRAIL_LEN], tz[TRAIL_LEN];
    int tlen;
    float atmR, atmG, atmB, atmA;
    bool hasRings;
};

struct Moon
{
    const char* name;
    float orb, sz, spd, ang, r, g, b, incl, ecc;
    const char* fact;
    float spinAngle, spinSpeed;
};

struct GalaxyStar
{
    float baseAngle, radius;
    float x, y, z;
    float brightness, twinklePhase, twinkleSpeed, size;
    int layer, colorType, armIndex;
    bool isCluster, isSFR;
};

struct Nebula
{
    float baseAngle, radius;
    float y, size, r, g, b, a, phase;
    float wx, wz;
    int type;
    float rotation;
};

struct SFR
{
    float wx, wz, y, size, phase;
    float baseAngle, radius;
};

struct Asteroid
{
    float ang, rad, ht, sz, spd, rot, rspd, r, g, b, elong;
    bool isCeres;
};

struct BlackHole
{
    float x, y, z, size, diskRadius, rotation, jetHeight;
    float lensRing;
};

struct Spacecraft
{
    float pathAngle, spd, x, y, z, yaw, pitch;
    float tx[TRAIL_LEN], ty[TRAIL_LEN], tz[TRAIL_LEN];
    int tlen;
    bool active;
    float batteryLevel, solarEfficiency, shieldLevel;
    float engineThrust;
    int missionDay;
    float bankAngle;
    float radarAngle;
    float engineHeat;
    float commSignal;
    float scanPhase;
};

struct CometParticle
{
    float x, y, z, vx, vy, vz, life, sz, r, g, b;
};

struct Comet
{
    float ang, spd, orbitR, ecc, incl;
    float x, y, z;
    CometParticle p[COMET_PARTS];
    int np;
    bool active;
    float tailLength;
    const char* name;
};

struct Meteor
{
    float x, y, z, vx, vy, vz;
    float life, maxLife, sz;
    bool active;
};

struct LensRay
{
    float angle, length, width, r, g, b, a;
};

struct DeepGalaxy
{
    float x, y, z;
    float scale;
    float r, g, b;
    float rotation;
    float spin;
    float flatten;
    int type;
    const char* name;
};

struct Quasar
{
    float x, y, z;
    float intensity;
    float phase;
    float r, g, b;
};

struct NearbySystem
{
    const char* name;
    const char* classification;
    const char* note;
    float dx, dy, dz;
    float wx, wy, wz;
    float size;
    float r, g, b;
    int exoplanets;
};

struct OuterBody
{
    const char* name;
    const char* kind;
    const char* note;
    float orbitRadius;
    float size;
    float orbitSpeed;
    float orbitAngle;
    float inclination;
    float eccentricity;
    float r, g, b;
    float wx, wy, wz;
};

static Planet planets[NP];
static Moon earthMoon;
static Moon jMoons[4];
static Moon titanMoon;
static Moon uranusMoons[4];
static Moon neptuneMoon;
static Moon plutoMoon;
static Moon marsMoons[2];
static GalaxyStar galaxyStars[NSTARS];
static Nebula nebulae[NNEB];
static SFR sfRegions[N_SFR];
static Asteroid asteroids[NAST];
static Spacecraft ship;
static Comet comet;
static BlackHole blackHole;
static Meteor meteors[N_METEORS];
static LensRay lensRays[N_LENS_RAYS];
static DeepGalaxy deepGalaxies[N_DEEP_GALAXIES];
static Quasar quasars[N_QUASARS];
static NearbySystem nearbySystems[N_NEARBY_SYSTEMS];
static OuterBody outerBodies[N_OUTER_BODIES];
static Camera cam;

static bool paused = false;
static bool showMap = true;
static bool showInfo = true;
static bool showPowerPanel = true;
static bool showTrails = true;
static bool showLabels = true;
static bool showScale = true;
static bool showClouds = true;
static bool showGrid = false;
static bool showArmNames = true;
static bool showClusters = true;
static bool showNebulae = true;
static bool showSFR = true;
static bool showOrbits = true;
static bool showLensFlare = true;
static bool showMeteors = true;
static bool showAsteroidBelt = true;
static bool showComet = true;
static bool showShip = true;
static bool showArmLines = true;
static bool nightMode = false;
static bool showHelp = true;
static bool showDepthDebug = false;
static bool showConstellationGuide = false;
static bool showGalaxyDiagnostics = false;
static bool showDeepSpace = true;
static bool showStarSurvey = false;
static bool showSystemOverview = false;
static bool showFocusPanel = false;
static bool showShipTelemetry = true;
static bool softFocusMode = true;
static float simSpeed = 1.0f;
static float sunTime = 0.0f;
static float galacticTime = 0.0f;
static float sunX = 0, sunY = 0, sunZ = 0;
static float sunGalacticAngle = 0.0f;
static const int solarSystemArm = 3;
static const float sunGalacticR = 200.0f;
static int infoPlanet = -1;
static int currentExploreIndex = 0;
static bool mouseDown = false;
static bool rightDown = false;
static int lastMouseX = 0, lastMouseY = 0;
static int mouseRX = 0, mouseRY = 0;
static int frameCounter = 0, fps = 0, fpsFrames = 0, lastFPSTime = 0;
static GLdouble gMV[16], gPR[16];
static GLint gVP[4];
static GLUquadric* Q = NULL;
static void startCameraTransition(float nd,float ny,float np2,float ntx,float nty,float ntz);
static float galacticOmega(float r);


static const char* HELP_LINES[N_HELP_LINES] = {
    "0 / 1-9  - focus Sun / planets",
    "B        - focus galactic core",
    "F / O    - follow / orbit mode",
    "P        - pause simulation",
    "+ / -    - simulation speed",
    "Tab      - cycle planet info",
    "Arrows   - rotate camera",
    "Mouse L  - rotate view",
    "Mouse R  - pan target",
    "Wheel    - zoom in / out",
    "M        - mini map",
    "I        - detailed planet info",
    "H        - power panel",
    "T / E    - trails / orbits",
    "L / S    - labels / scale HUD",
    "C / G    - clouds / grid",
    "N / X    - arm names / guide lines",
    "K / V / Z- clusters / nebulae / SFR",
    "Y / U / Q- meteors / ship / comet",
    "W / R    - night mode / reset",
    "[ / ]    - cycle beyond targets",
    "\\       - exploration panel",
    "; / A    - deep space / diagnostics",
    ",        - ship telemetry",
    "D / `    - depth / strategy",
    "J        - show / hide control panel"
};

static float frand01() { return (float)std::rand() / (float)RAND_MAX; }
static float frandRange(float a, float b) { return a + (b - a) * frand01(); }
static float smoothstepf(float t) { t = CLAMP(t, 0, 1); return t * t * (3 - 2 * t); }
static float smootherstepf(float t) { t = CLAMP(t, 0, 1); return t * t * t * (t * (t * 6 - 15) + 10); }

static float gaussRand()
{
    float u1 = frand01();
    if(u1 < 0.0001f) u1 = 0.0001f;
    return std::sqrt(-2.0f * std::log(u1)) * std::cos(TWO_PI * frand01());
}

static float vecLen(float x, float y, float z)
{
    return std::sqrt(x*x + y*y + z*z);
}

static void vecNorm(float& x, float& y, float& z)
{
    float l = vecLen(x, y, z);
    if(l > 0.0001f) {
        x /= l;
        y /= l;
        z /= l;
    }
}

static void beginOrtho()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

static void endOrtho()
{
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

static float depthFade(float x, float y, float z, float nearD, float farD)
{
    float dx = x - cam.smoothX;
    float dy = y - cam.smoothY;
    float dz = z - cam.smoothZ;
    float d  = std::sqrt(dx*dx + dy*dy + dz*dz);
    return 1.0f - CLAMP((d - nearD) / (farD - nearD), 0.0f, 1.0f);
}

static const int FOCUS_SUN = -1;
static const int FOCUS_BLACKHOLE = -2;
static const int FOCUS_NEARBY_BASE = -100;
static const int FOCUS_DEEPGALAXY_BASE = -200;
static const int FOCUS_SHIP = -300;
static const int FOCUS_COMET = -301;
static const int FOCUS_OUTER_BASE = -400;

static void focusOnPlanet(int idx);
static bool focusIsNearby();
static bool focusIsDeepGalaxy();
static bool focusIsOuterBody();
static bool focusIsShip();
static bool focusIsComet();
static int  focusNearbyIndex();
static int  focusDeepGalaxyIndex();
static int  focusOuterIndex();

static bool getCurrentFocusWorld(float& fx, float& fy, float& fz)
{
    if(cam.focusPlanet >= 0 && cam.focusPlanet < NP) {
        fx = planets[cam.focusPlanet].wx;
        fy = planets[cam.focusPlanet].wy;
        fz = planets[cam.focusPlanet].wz;
        return true;
    }
    if(cam.focusPlanet == FOCUS_SUN) {
        fx = sunX; fy = sunY; fz = sunZ;
        return true;
    }
    if(cam.focusPlanet == FOCUS_BLACKHOLE) {
        fx = blackHole.x; fy = blackHole.y; fz = blackHole.z;
        return true;
    }
    if(cam.focusPlanet == FOCUS_SHIP) {
        fx = ship.x; fy = ship.y; fz = ship.z;
        return true;
    }
    if(cam.focusPlanet == FOCUS_COMET) {
        fx = comet.x; fy = comet.y; fz = comet.z;
        return true;
    }
    if(focusIsNearby()) {
        int idx = focusNearbyIndex();
        if(idx >= 0 && idx < N_NEARBY_SYSTEMS) {
            fx = nearbySystems[idx].wx; fy = nearbySystems[idx].wy; fz = nearbySystems[idx].wz;
            return true;
        }
    }
    if(focusIsOuterBody()) {
        int idx = focusOuterIndex();
        if(idx >= 0 && idx < N_OUTER_BODIES) {
            fx = outerBodies[idx].wx; fy = outerBodies[idx].wy; fz = outerBodies[idx].wz;
            return true;
        }
    }
    if(focusIsDeepGalaxy()) {
        int idx = focusDeepGalaxyIndex();
        if(idx >= 0 && idx < N_DEEP_GALAXIES) {
            fx = deepGalaxies[idx].x; fy = deepGalaxies[idx].y; fz = deepGalaxies[idx].z;
            return true;
        }
    }
    return false;
}

static float focusWeight(float x, float y, float z)
{
    if(!softFocusMode) return 1.0f;
    float fx=0.0f, fy=0.0f, fz=0.0f;
    if(!getCurrentFocusWorld(fx, fy, fz)) return 1.0f;
    float d  = DIST2(x, y, z, fx, fy, fz);
    return 0.30f + 0.70f * (1.0f - CLAMP(d / 45.0f, 0.0f, 1.0f));
}

static float zoomGate(float nearZ, float farZ)
{
    return CLAMP((cam.dist - nearZ) / (farZ - nearZ), 0.0f, 1.0f);
}

static bool isFarView() { return cam.dist > 140.0f; }
static bool isCloseView() { return cam.dist < 18.0f; }
static bool isPlanetView() { return cam.focusPlanet >= 0 && cam.dist < 24.0f; }

static bool focusIsNearby()    { return cam.focusPlanet <= FOCUS_NEARBY_BASE    && cam.focusPlanet > FOCUS_DEEPGALAXY_BASE; }
static bool focusIsDeepGalaxy(){ return cam.focusPlanet <= FOCUS_DEEPGALAXY_BASE && cam.focusPlanet > FOCUS_SHIP; }
static bool focusIsOuterBody() { return cam.focusPlanet <= FOCUS_OUTER_BASE     && cam.focusPlanet > (FOCUS_OUTER_BASE - 100); }
static bool focusIsShip()      { return cam.focusPlanet == FOCUS_SHIP; }
static bool focusIsComet()     { return cam.focusPlanet == FOCUS_COMET; }
static int  focusNearbyIndex() { return FOCUS_NEARBY_BASE - cam.focusPlanet; }
static int  focusDeepGalaxyIndex() { return FOCUS_DEEPGALAXY_BASE - cam.focusPlanet; }
static int  focusOuterIndex()  { return FOCUS_OUTER_BASE - cam.focusPlanet; }

static const char* currentFocusLabel()
{
    if(cam.focusPlanet >= 0 && cam.focusPlanet < NP) return planets[cam.focusPlanet].name;
    if(cam.focusPlanet == FOCUS_SUN) return "Sun";
    if(cam.focusPlanet == FOCUS_BLACKHOLE) return "Sagittarius A*";
    if(cam.focusPlanet == FOCUS_SHIP) return "Voyager-I";
    if(cam.focusPlanet == FOCUS_COMET) return comet.name;
    if(focusIsNearby()) {
        int idx = focusNearbyIndex();
        if(idx >= 0 && idx < N_NEARBY_SYSTEMS) return nearbySystems[idx].name;
    }
    if(focusIsOuterBody()) {
        int idx = focusOuterIndex();
        if(idx >= 0 && idx < N_OUTER_BODIES) return outerBodies[idx].name;
    }
    if(focusIsDeepGalaxy()) {
        int idx = focusDeepGalaxyIndex();
        if(idx >= 0 && idx < N_DEEP_GALAXIES) return deepGalaxies[idx].name;
    }
    return "Free Camera";
}

static const char* currentFocusCategory()
{
    if(cam.focusPlanet >= 0 && cam.focusPlanet < NP) return "Solar Planet";
    if(cam.focusPlanet == FOCUS_SUN) return "Star";
    if(cam.focusPlanet == FOCUS_BLACKHOLE) return "Galactic Core";
    if(cam.focusPlanet == FOCUS_SHIP) return "Spacecraft";
    if(cam.focusPlanet == FOCUS_COMET) return "Comet";
    if(focusIsNearby()) return "Nearby Star";
    if(focusIsOuterBody()) return "Outer Frontier";
    if(focusIsDeepGalaxy()) return "Deep Galaxy";
    return "Free Camera";
}

static void currentFocusColor(float& r,float& g,float& b)
{
    if(cam.focusPlanet >= 0 && cam.focusPlanet < NP) {
        r = planets[cam.focusPlanet].r; g = planets[cam.focusPlanet].g; b = planets[cam.focusPlanet].b; return;
    }
    if(cam.focusPlanet == FOCUS_SUN) { r = 1.0f; g = 0.92f; b = 0.28f; return; }
    if(cam.focusPlanet == FOCUS_BLACKHOLE) { r = 0.88f; g = 0.66f; b = 1.0f; return; }
    if(cam.focusPlanet == FOCUS_SHIP) { r = 0.30f; g = 0.92f; b = 1.0f; return; }
    if(cam.focusPlanet == FOCUS_COMET) { r = 0.80f; g = 0.90f; b = 1.0f; return; }
    if(focusIsNearby()) {
        int idx = focusNearbyIndex();
        if(idx >= 0 && idx < N_NEARBY_SYSTEMS) { r = nearbySystems[idx].r; g = nearbySystems[idx].g; b = nearbySystems[idx].b; return; }
    }
    if(focusIsOuterBody()) {
        int idx = focusOuterIndex();
        if(idx >= 0 && idx < N_OUTER_BODIES) { r = outerBodies[idx].r; g = outerBodies[idx].g; b = outerBodies[idx].b; return; }
    }
    if(focusIsDeepGalaxy()) {
        int idx = focusDeepGalaxyIndex();
        if(idx >= 0 && idx < N_DEEP_GALAXIES) { r = deepGalaxies[idx].r; g = deepGalaxies[idx].g; b = deepGalaxies[idx].b; return; }
    }
    r = 0.86f; g = 0.90f; b = 1.0f;
}

static int nearestArmIndexForPoint(float wx,float wz)
{
    float r = std::sqrt(wx*wx + wz*wz);
    if(r < 20.0f) return solarSystemArm;
    float angle = std::atan2(wz, wx);
    int bestArm = solarSystemArm;
    float bestDiff = 9999.0f;
    for(int arm=0; arm<N_GALAXY_ARMS; ++arm) {
        float armBase = (arm / (float)N_GALAXY_ARMS) * TWO_PI;
        float curve = armBase + 0.55f * std::log(r + 1.0f) + galacticTime * galacticOmega(r);
        float diff = std::fabs(std::atan2(std::sin(angle - curve), std::cos(angle - curve)));
        if(diff < bestDiff) {
            bestDiff = diff;
            bestArm = arm;
        }
    }
    return bestArm;
}

static int currentFocusArmIndex()
{
    if(cam.focusPlanet == FOCUS_BLACKHOLE) return -1;
    if(focusIsDeepGalaxy()) return -1;
    float fx, fy, fz;
    if(!getCurrentFocusWorld(fx, fy, fz)) return solarSystemArm;
    return nearestArmIndexForPoint(fx, fz);
}

static bool isWorldCurrentFocus(float wx,float wy,float wz,float radius)
{
    float fx, fy, fz;
    if(!getCurrentFocusWorld(fx, fy, fz)) return false;
    return DIST2(wx, wy, wz, fx, fy, fz) <= radius;
}

static void getStarColor(int ct, float& r, float& g, float& b)
{
    switch(ct) {
        case 0: r=0.55f; g=0.65f; b=0.95f; break;
        case 1: r=0.75f; g=0.82f; b=1.00f; break;
        case 2: r=0.95f; g=0.95f; b=1.00f; break;
        case 3: r=1.00f; g=0.96f; b=0.84f; break;
        case 4: r=1.00f; g=0.88f; b=0.55f; break;
        case 5: r=1.00f; g=0.70f; b=0.35f; break;
        default:r=1.00f; g=0.45f; b=0.25f; break;
    }
}

static void setLightAtSun()
{
    GLfloat lp[] = { sunX, sunY, sunZ, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lp);
}

static void initPlanetData()
{
    planets[0] = {"Mercury","No atmosphere. Surface temp swings 600C.",
        "Smallest planet. Heavily cratered surface.","Iron core, silicate mantle",
        3.0f,0.12f,4.74f,0.07f,0,0,0.03f,0.205f,
        0.70f,0.60f,0.50f,10,
        4879,57.9f,88,1407.6f,3.7f,167,0,
        0,0,0,{0},{0},{0},0,
        0,0,0,0,false};

    planets[1] = {"Venus","Hottest planet despite not being closest.",
        "Dense CO2 atmosphere creates runaway greenhouse effect.","Iron core, rocky mantle, thick CO2 atmosphere",
        4.2f,0.20f,1.86f,-0.02f,40,0,177.0f,0.007f,
        0.92f,0.72f,0.42f,20,
        12104,108.2f,225,5832.0f,8.87f,464,0,
        0,0,0,{0},{0},{0},0,
        0.95f,0.80f,0.50f,0.14f,false};

    planets[2] = {"Earth","Only known world with liquid surface water.",
        "Magnetic field, plate tectonics, complex biosphere.","Iron/nickel core, silicate mantle, N2/O2 atmosphere",
        6.0f,0.22f,1.14f,1.00f,90,0,23.5f,0.017f,
        0.20f,0.50f,0.92f,30,
        12756,149.6f,365,24.0f,9.81f,15,1,
        0,0,0,{0},{0},{0},0,
        0.35f,0.60f,1.0f,0.18f,false};

    planets[3] = {"Mars","Hosts Olympus Mons, largest volcano in solar system.",
        "Cold dusty world. Thin CO2 atmosphere.","Iron core, basaltic mantle, CO2 atmosphere",
        7.8f,0.16f,0.61f,0.97f,150,0,25.0f,0.093f,
        0.82f,0.35f,0.15f,10,
        6792,227.9f,687,24.6f,3.72f,-65,2,
        0,0,0,{0},{0},{0},0,
        0.90f,0.45f,0.25f,0.05f,false};

    planets[4] = {"Jupiter","Largest planet - mass exceeds all others combined.",
        "Great Red Spot has raged for centuries.","Hydrogen/helium gas giant, metallic hydrogen core",
        11.0f,0.70f,0.10f,2.40f,210,0,3.1f,0.049f,
        0.84f,0.72f,0.52f,15,
        142984,778.6f,4333,9.9f,24.79f,-110,95,
        0,0,0,{0},{0},{0},0,
        0.85f,0.75f,0.55f,0.06f,false};

    planets[5] = {"Saturn","Least dense planet - would float on water.",
        "Ring system spans vast distances but is surprisingly thin.","Hydrogen/helium gas giant, rocky/ice core",
        15.0f,0.60f,0.04f,2.20f,270,0,26.7f,0.057f,
        0.92f,0.82f,0.58f,25,
        120536,1433.5f,10759,10.7f,10.44f,-140,146,
        0,0,0,{0},{0},{0},0,
        0.88f,0.75f,0.42f,0.05f,true};

    planets[6] = {"Uranus","Rotates on its side - axial tilt 97.8 deg.",
        "Ice giant with a faint dark ring system.","Water/methane/ammonia ice mantle, rocky core",
        19.0f,0.40f,0.014f,1.40f,310,0,97.8f,0.046f,
        0.45f,0.82f,0.92f,20,
        51118,2872.5f,30687,17.2f,8.87f,-195,27,
        0,0,0,{0},{0},{0},0,
        0.45f,0.85f,0.95f,0.10f,true};

    planets[7] = {"Neptune","Winds reach extreme speeds.",
        "Deep blue from methane absorption of red light.","Water/methane ice, rocky core, thin ring arcs",
        22.0f,0.38f,0.006f,1.50f,350,0,28.3f,0.010f,
        0.25f,0.35f,0.85f,20,
        49528,4495.1f,60190,16.1f,11.15f,-200,16,
        0,0,0,{0},{0},{0},0,
        0.25f,0.40f,0.95f,0.10f,false};

    planets[8] = {"Pluto","Reclassified as dwarf planet in 2006.",
        "Heart-shaped Tombaugh Regio made Pluto iconic.","Rock and ice with nitrogen, CO, methane ices",
        26.0f,0.10f,0.004f,0.50f,10,0,122.0f,0.248f,
        0.85f,0.80f,0.75f,5,
        2377,5906.4f,90560,153.3f,0.62f,-229,5,
        0,0,0,{0},{0},{0},0,
        0.70f,0.70f,0.80f,0.03f,false};
}

static void initMoonData()
{
    earthMoon = {"Moon",0.65f,0.08f,13.17f,0,0.80f,0.80f,0.80f,5.1f,0.055f,"Tidally locked to Earth.",0,3.0f};
    jMoons[0] = {"Io",1.10f,0.08f,20.0f,0,0.95f,0.80f,0.30f,0.04f,0.004f,"Most volcanically active body.",0,2.5f};
    jMoons[1] = {"Europa",1.40f,0.07f,10.0f,90,0.80f,0.85f,1.0f,0.47f,0.009f,"Subsurface ocean candidate.",0,1.8f};
    jMoons[2] = {"Ganymede",1.80f,0.09f,5.0f,180,0.75f,0.65f,0.50f,0.18f,0.001f,"Largest moon in the solar system.",0,1.5f};
    jMoons[3] = {"Callisto",2.20f,0.08f,2.0f,270,0.55f,0.45f,0.45f,0.19f,0.007f,"Ancient cratered surface.",0,1.2f};
    titanMoon = {"Titan",1.30f,0.09f,8.0f,0,0.92f,0.72f,0.35f,0.33f,0.029f,"Methane lakes and thick atmosphere.",0,2.0f};
    marsMoons[0] = {"Phobos",0.45f,0.04f,30.0f,0,0.55f,0.50f,0.45f,1.1f,0.015f,"Doomed moon spiraling inward.",0,4.0f};
    marsMoons[1] = {"Deimos",0.70f,0.03f,12.0f,90,0.50f,0.48f,0.42f,0.9f,0.002f,"Slowly drifting away.",0,2.5f};
    uranusMoons[0]= {"Miranda",0.75f,0.05f,15.0f,0,0.65f,0.70f,0.75f,4.3f,0.001f,"Patchwork terrain.",0,2.0f};
    uranusMoons[1]= {"Ariel",0.95f,0.06f,10.0f,90,0.60f,0.65f,0.72f,0.04f,0.001f,"Bright icy surface.",0,1.8f};
    uranusMoons[2]= {"Umbriel",1.20f,0.06f,6.0f,180,0.45f,0.45f,0.50f,0.13f,0.004f,"Darkest major Uranian moon.",0,1.5f};
    uranusMoons[3]= {"Titania",1.50f,0.07f,3.5f,270,0.55f,0.60f,0.65f,0.08f,0.001f,"Largest moon of Uranus.",0,1.3f};
    neptuneMoon = {"Triton",0.95f,0.07f,12.0f,45,0.70f,0.75f,0.85f,157.0f,0.00002f,"Retrograde captured world.",0,2.0f};
    plutoMoon = {"Charon",0.50f,0.06f,8.0f,0,0.70f,0.70f,0.75f,0.0f,0.0f,"Binary dwarf companion.",0,1.5f};
}

static int addStar(int idx,float baseAngle,float radius,float y,float brightness,int colorType,float sz,int layer,int arm,bool cluster,bool sfr)
{
    if(idx >= NSTARS) return idx;
    GalaxyStar& s = galaxyStars[idx];
    s.baseAngle = baseAngle;
    s.radius = radius;
    s.y = y;
    s.x = std::cos(baseAngle) * radius;
    s.z = std::sin(baseAngle) * radius;
    s.brightness = brightness;
    s.twinklePhase = frandRange(0, TWO_PI);
    s.twinkleSpeed = frandRange(0.3f, 4.0f);
    s.colorType = colorType;
    s.size = sz;
    s.layer = layer;
    s.armIndex = arm;
    s.isCluster = cluster;
    s.isSFR = sfr;
    return idx + 1;
}

static float galacticOmega(float r)
{
    if(r < 1.0f) return 0.0f;
    return 1.0f / (r * 0.0045f + 0.35f);
}

static void generateSpiralGalaxy()
{
    int idx = 0;

    for(int i=0;i<N_BACKGROUND_STARS && idx<NSTARS;i++) {
        float th = frandRange(0, TWO_PI);
        float ph = frandRange(0, PI);
        float r = frandRange(700, 1400);
        GalaxyStar& s = galaxyStars[idx];
        s.baseAngle = th;
        s.radius = 0;
        s.x = r * std::sin(ph) * std::cos(th);
        s.y = r * std::cos(ph);
        s.z = r * std::sin(ph) * std::sin(th);
        s.brightness = frandRange(0.08f, 0.45f);
        s.twinklePhase = frandRange(0, TWO_PI);
        s.twinkleSpeed = frandRange(0.2f, 2.0f);
        s.colorType = std::rand() % 7;
        s.size = frandRange(0.8f, 1.5f);
        s.layer = 0;
        s.armIndex = -1;
        s.isCluster = false;
        s.isSFR = false;
        idx++;
    }

    for(int i=0;i<N_HALO_STARS && idx<NSTARS;i++) {
        float r = frandRange(60, 380);
        float th = frandRange(0, TWO_PI);
        float ph = frandRange(0, PI);
        float x = r * std::sin(ph) * std::cos(th);
        float y = r * std::cos(ph) * 0.55f;
        float z = r * std::sin(ph) * std::sin(th);
        idx = addStar(idx, std::atan2(z, x), r, y, frandRange(0.12f, 0.42f), std::rand()%4, frandRange(0.5f, 1.1f), 5, -1, false, false);
    }

    for(int i=0;i<N_CORE_STARS && idx<NSTARS;i++) {
        float r = std::abs(gaussRand()) * 42.0f;
        float angle = frandRange(0, TWO_PI);
        float y = gaussRand() * 16.0f * (1.0f - r / 65.0f);
        int ct = (r < 18) ? std::rand()%3 : std::rand()%5;
        idx = addStar(idx, angle, r, y, frandRange(0.55f, 1.0f), ct, frandRange(1.4f, 3.8f), 2, -1, false, false);
    }

    for(int i=0;i<N_BAR_STARS && idx<NSTARS;i++) {
        float r = frandRange(0, 60);
        float barAngle = DEG2RAD(30.0f);
        float perpSpread = gaussRand() * 8.0f;
        float x = (r + perpSpread*0.3f) * std::cos(barAngle) - perpSpread * std::sin(barAngle);
        float z = (r + perpSpread*0.3f) * std::sin(barAngle) + perpSpread * std::cos(barAngle);
        float y = gaussRand() * 5.0f;
        idx = addStar(idx, std::atan2(z,x), std::sqrt(x*x+z*z), y, frandRange(0.45f,0.90f), std::rand()%5, frandRange(1.2f,2.8f), 2, -1, false, false);
        if(idx < NSTARS) {
            idx = addStar(idx, std::atan2(-z,-x), std::sqrt(x*x+z*z), gaussRand()*5.0f, frandRange(0.45f,0.90f), std::rand()%5, frandRange(1.2f,2.8f), 2, -1, false, false);
        }
    }

    for(int arm=0; arm<N_GALAXY_ARMS; arm++) {
        float armBase = (arm / (float)N_GALAXY_ARMS) * TWO_PI;
        for(int i=0;i<STARS_PER_ARM && idx<NSTARS;i++) {
            float t = (float)i / STARS_PER_ARM;
            float r = 28.0f + t * 460.0f;
            float spiralA = armBase + 0.55f * std::log(r + 1.0f);
            float spread = 5.0f + t * 52.0f;
            float perp = gaussRand() * spread * 0.20f;
            float angOff = perp / (r + 15.0f);
            float finalA = spiralA + angOff;
            float y = gaussRand() * (2.5f + t*11.0f) * 0.25f;
            float bright = frandRange(0.18f, 0.96f) * (1.0f - t*0.10f);
            int ct = (t<0.2f) ? std::rand()%4 : (t<0.55f ? std::rand()%6 : std::rand()%7);
            bool sfr = (t<0.6f && frand01()<0.004f);
            idx = addStar(idx, finalA, r, y, bright, ct, frandRange(0.9f,2.5f), 1, arm, false, sfr);
        }

        int cpa = N_ARM_CLUSTERS / N_GALAXY_ARMS;
        for(int c=0;c<cpa && idx<NSTARS;c++) {
            float t = frandRange(0.12f, 0.88f);
            float r = 35.0f + t * 420.0f;
            float spiralA = armBase + 0.55f * std::log(r + 1.0f);
            float cy = gaussRand() * 3.5f;
            for(int s=0;s<STARS_PER_CLUSTER && idx<NSTARS;s++) {
                float dr = gaussRand() * 5.5f;
                float da = gaussRand() * 5.5f / (r + 1.0f);
                float dy = gaussRand() * 2.5f;
                idx = addStar(idx, spiralA+da, r+dr, cy+dy, frandRange(0.45f,1.0f), std::rand()%6, frandRange(1.1f,2.4f), 3, arm, true, false);
            }
        }
    }

    for(int i=0;i<N_DUST_LANES && idx<NSTARS;i++) {
        int arm = std::rand() % N_GALAXY_ARMS;
        float armBase = (arm / (float)N_GALAXY_ARMS) * TWO_PI;
        float t = frandRange(0.08f, 0.92f);
        float r = 28.0f + t * 440.0f;
        float spiralA = armBase + 0.55f * std::log(r + 1.0f);
        float offset = frandRange(-0.07f, 0.07f);
        idx = addStar(idx, spiralA+offset, r, gaussRand()*1.8f, frandRange(0.04f,0.18f), 3, frandRange(0.4f,0.9f), 4, arm, false, false);
    }

    for(int i=0;i<N_STREAM_STARS && idx<NSTARS;i++) {
        float streamAngle = frandRange(0, TWO_PI);
        float streamR = frandRange(120, 280);
        float arcSpread = frandRange(0.0f, 0.6f);
        float finalA = streamAngle + arcSpread;
        float y = gaussRand() * 8.0f;
        idx = addStar(idx, finalA, streamR, y, frandRange(0.10f,0.35f), std::rand()%5, frandRange(0.5f,1.2f), 6, -1, false, false);
    }
}

static void initNebulae()
{
    std::srand(137);
    for(int i=0;i<NNEB;i++) {
        float th = frandRange(0, TWO_PI);
        float r = frandRange(25, 450);
        float spiralA = th + 0.55f * std::log(r + 1.0f);
        nebulae[i].baseAngle = spiralA;
        nebulae[i].radius = r;
        nebulae[i].y = gaussRand() * 14.0f;
        nebulae[i].wx = std::cos(spiralA)*r + gaussRand()*20.0f;
        nebulae[i].wz = std::sin(spiralA)*r + gaussRand()*20.0f;
        nebulae[i].size = frandRange(18, 70);
        nebulae[i].phase = frandRange(0, TWO_PI);
        nebulae[i].rotation = frandRange(0, 360);
        nebulae[i].type = i % 3;
        switch(i % 11) {
            case 0: nebulae[i].r=0.90f; nebulae[i].g=0.20f; nebulae[i].b=0.35f; break;
            case 1: nebulae[i].r=0.30f; nebulae[i].g=0.50f; nebulae[i].b=0.95f; break;
            case 2: nebulae[i].r=0.85f; nebulae[i].g=0.38f; nebulae[i].b=0.10f; break;
            case 3: nebulae[i].r=0.20f; nebulae[i].g=0.80f; nebulae[i].b=0.50f; break;
            case 4: nebulae[i].r=0.72f; nebulae[i].g=0.15f; nebulae[i].b=0.85f; break;
            case 5: nebulae[i].r=1.00f; nebulae[i].g=0.55f; nebulae[i].b=0.20f; break;
            case 6: nebulae[i].r=0.15f; nebulae[i].g=0.25f; nebulae[i].b=0.80f; break;
            case 7: nebulae[i].r=0.85f; nebulae[i].g=0.20f; nebulae[i].b=0.50f; break;
            case 8: nebulae[i].r=0.60f; nebulae[i].g=0.85f; nebulae[i].b=0.40f; break;
            case 9: nebulae[i].r=0.95f; nebulae[i].g=0.90f; nebulae[i].b=0.50f; break;
            default:nebulae[i].r=0.50f; nebulae[i].g=0.40f; nebulae[i].b=0.70f; break;
        }
        nebulae[i].a = frandRange(0.004f, 0.024f);
    }

    std::srand(777);
    for(int i=0;i<N_SFR;i++) {
        int arm = std::rand() % N_GALAXY_ARMS;
        float armBase = (arm / (float)N_GALAXY_ARMS) * TWO_PI;
        float t = frandRange(0.15f, 0.80f);
        float r = 35.0f + t * 400.0f;
        float spiralA = armBase + 0.55f * std::log(r + 1.0f);
        sfRegions[i].baseAngle = spiralA;
        sfRegions[i].radius = r;
        sfRegions[i].wx = std::cos(spiralA) * r;
        sfRegions[i].wz = std::sin(spiralA) * r;
        sfRegions[i].y = gaussRand() * 3.0f;
        sfRegions[i].size = frandRange(3.0f, 10.0f);
        sfRegions[i].phase = frandRange(0, TWO_PI);
    }
}

static void initMeteors()
{
    for(int i=0;i<N_METEORS;i++) meteors[i].active = false;
}

static void spawnMeteor()
{
    for(int i=0;i<N_METEORS;i++) {
        if(!meteors[i].active) {
            meteors[i].x = frandRange(-600, 600);
            meteors[i].y = frandRange(-200, 300);
            meteors[i].z = frandRange(-600, 600);
            float speed = frandRange(15, 45);
            float angle = frandRange(0, TWO_PI);
            float dive = frandRange(-0.3f, 0.3f);
            meteors[i].vx = std::cos(angle) * speed;
            meteors[i].vy = dive * speed;
            meteors[i].vz = std::sin(angle) * speed;
            meteors[i].maxLife = frandRange(0.5f, 1.8f);
            meteors[i].life = meteors[i].maxLife;
            meteors[i].sz = frandRange(0.5f, 2.0f);
            meteors[i].active = true;
            return;
        }
    }
}

static void initLensFlare()
{
    for(int i=0;i<N_LENS_RAYS;i++) {
        lensRays[i].angle = i * TWO_PI / N_LENS_RAYS + frandRange(-0.2f, 0.2f);
        lensRays[i].length = frandRange(20, 80);
        lensRays[i].width = frandRange(1.0f, 3.0f);
        float hue = frandRange(0.0f, 1.0f);
        if(hue < 0.4f) {
            lensRays[i].r = 1.0f; lensRays[i].g = 0.8f + hue; lensRays[i].b = 0.3f;
        } else if(hue < 0.7f) {
            lensRays[i].r = 0.7f; lensRays[i].g = 0.9f; lensRays[i].b = 1.0f;
        } else {
            lensRays[i].r = 1.0f; lensRays[i].g = 0.5f; lensRays[i].b = 0.8f;
        }
        lensRays[i].a = frandRange(0.06f, 0.20f);
    }
}

static void initAsteroids()
{
    for(int i=0;i<NAST;i++) {
        asteroids[i].ang = frandRange(0, 360);
        asteroids[i].rad = frandRange(8.0f, 10.5f);
        asteroids[i].ht = frandRange(-0.55f, 0.55f);
        asteroids[i].sz = frandRange(0.012f, 0.085f);
        asteroids[i].spd = frandRange(0.12f, 0.58f);
        asteroids[i].rot = frandRange(0, 360);
        asteroids[i].rspd = frandRange(0.4f, 5.0f);
        asteroids[i].r = frandRange(0.42f, 0.72f);
        asteroids[i].g = frandRange(0.35f, 0.62f);
        asteroids[i].b = frandRange(0.28f, 0.55f);
        asteroids[i].elong = frandRange(1.0f, 2.4f);
        asteroids[i].isCeres = (i == 0);
    }
}

static void initDeepSpace()
{
    static const char* names[N_DEEP_GALAXIES] = {
        "Andromeda Analog","Triangulum Echo","Sombrero Veil","Whirlpool Ghost",
        "Centaurus Lantern","Cartwheel Ember","Pinwheel Bloom","Mice Pair",
        "Antennae Drift","Sculptor Halo","Virgo Arc","Fornax Candle",
        "Pegasus Tide","Cetus Swirl","Lyra Wheel","Hydra Haze",
        "Phoenix Disc","Draco Lens"
    };
    std::srand(31415);
    for(int i=0;i<N_DEEP_GALAXIES;i++) {
        float th = frandRange(0.0f, TWO_PI);
        float ph = frandRange(0.15f, PI-0.15f);
        float rr = frandRange(900.0f, 2200.0f);
        deepGalaxies[i].x = rr * std::sin(ph) * std::cos(th);
        deepGalaxies[i].y = rr * std::cos(ph) * 0.65f;
        deepGalaxies[i].z = rr * std::sin(ph) * std::sin(th);
        deepGalaxies[i].scale = frandRange(18.0f, 54.0f);
        deepGalaxies[i].r = frandRange(0.45f, 1.00f);
        deepGalaxies[i].g = frandRange(0.45f, 0.95f);
        deepGalaxies[i].b = frandRange(0.55f, 1.00f);
        deepGalaxies[i].rotation = frandRange(0.0f, 360.0f);
        deepGalaxies[i].spin = frandRange(-0.12f, 0.12f);
        deepGalaxies[i].flatten = frandRange(0.18f, 0.58f);
        deepGalaxies[i].type = i % 3;
        deepGalaxies[i].name = names[i];
    }
    std::srand(27182);
    for(int i=0;i<N_QUASARS;i++) {
        float th = frandRange(0.0f, TWO_PI);
        float ph = frandRange(0.2f, PI-0.2f);
        float rr = frandRange(1300.0f, 2600.0f);
        quasars[i].x = rr * std::sin(ph) * std::cos(th);
        quasars[i].y = rr * std::cos(ph) * 0.75f;
        quasars[i].z = rr * std::sin(ph) * std::sin(th);
        quasars[i].intensity = frandRange(0.45f, 1.0f);
        quasars[i].phase = frandRange(0.0f, TWO_PI);
        quasars[i].r = frandRange(0.60f, 1.0f);
        quasars[i].g = frandRange(0.65f, 1.0f);
        quasars[i].b = 1.0f;
    }
}

static void initNearbySystems()
{
    nearbySystems[0] = {"Proxima Centauri", "M-type red dwarf", "Nearest known stellar neighbor with rocky worlds in the habitable zone discussion.", 62.0f, 4.0f, 28.0f, 0,0,0, 0.16f, 1.00f, 0.54f, 0.40f, 3};
    nearbySystems[1] = {"Alpha Centauri A", "G-type main sequence", "Sun-like star in the closest multi-star system.", 58.0f, 3.0f, 34.0f, 0,0,0, 0.22f, 1.00f, 0.92f, 0.66f, 0};
    nearbySystems[2] = {"Alpha Centauri B", "K-type orange dwarf", "Companion star with warmer amber hue and strong visual contrast.", 55.0f, 2.0f, 38.0f, 0,0,0, 0.19f, 1.00f, 0.72f, 0.38f, 1};
    nearbySystems[3] = {"Barnard's Star", "M-type red dwarf", "Fast proper-motion star with a quiet, lonely character.", 86.0f, 6.0f, -18.0f, 0,0,0, 0.15f, 1.00f, 0.46f, 0.38f, 1};
    nearbySystems[4] = {"Sirius A", "A-type white star", "Bright beacon representing luminous stars beyond the local Sun-centered view.", -74.0f, 10.0f, 42.0f, 0,0,0, 0.28f, 0.85f, 0.92f, 1.00f, 0};
    nearbySystems[5] = {"TRAPPIST-1", "Ultra-cool red dwarf", "Compact exoplanet system famous for tightly packed terrestrial planets.", -92.0f, -4.0f, -26.0f, 0,0,0, 0.13f, 1.00f, 0.42f, 0.34f, 7};
    nearbySystems[6] = {"Kepler-452 Beacon", "G-type analog marker", "A teaching marker for exoplanet exploration beyond the Solar System.", 118.0f, 14.0f, 16.0f, 0,0,0, 0.18f, 0.95f, 0.88f, 0.62f, 1};
}

static void initOuterBodies()
{
    outerBodies[0] = {"Haumea", "dwarf planet", "Rapidly spinning icy world in the Kuiper Belt.", 34.0f, 0.08f, 0.022f, 20.0f, 28.0f, 0.19f, 0.80f, 0.84f, 0.90f, 0,0,0};
    outerBodies[1] = {"Makemake", "dwarf planet", "Reddish methane-ice dwarf beyond Pluto.", 39.0f, 0.09f, 0.018f, 140.0f, 29.0f, 0.16f, 0.88f, 0.72f, 0.54f, 0,0,0};
    outerBodies[2] = {"Eris", "scattered-disk dwarf", "Massive outer dwarf on a cold distant path.", 46.0f, 0.10f, 0.014f, 250.0f, 44.0f, 0.44f, 0.86f, 0.88f, 0.94f, 0,0,0};
    outerBodies[3] = {"Sedna", "inner Oort candidate", "Extreme trans-Neptunian world with a very elongated remote orbit.", 68.0f, 0.07f, 0.007f, 310.0f, 12.0f, 0.74f, 0.92f, 0.54f, 0.42f, 0,0,0};
    outerBodies[4] = {"Planet Nine Hypothesis", "theoretical world", "A speculative distant giant used as an exploration placeholder beyond the classical planets.", 118.0f, 0.18f, 0.003f, 70.0f, 18.0f, 0.26f, 0.42f, 0.58f, 0.92f, 0,0,0};
}

static int totalExploreTargets()
{
    return N_NEARBY_SYSTEMS + N_OUTER_BODIES + 2 + 4;
}

static void focusOnNearbySystem(int idx)
{
    if(idx < 0 || idx >= N_NEARBY_SYSTEMS) return;
    cam.focusPlanet = FOCUS_NEARBY_BASE - idx;
    cam.mode = CAM_FREE;
    startCameraTransition(10.0f + nearbySystems[idx].size * 70.0f, 38.0f, 18.0f, nearbySystems[idx].wx, nearbySystems[idx].wy, nearbySystems[idx].wz);
    infoPlanet = -1;
}

static void focusOnOuterBody(int idx)
{
    if(idx < 0 || idx >= N_OUTER_BODIES) return;
    cam.focusPlanet = FOCUS_OUTER_BASE - idx;
    cam.mode = CAM_FREE;
    startCameraTransition(outerBodies[idx].size * 22.0f + 3.0f, 36.0f, 18.0f, outerBodies[idx].wx, outerBodies[idx].wy, outerBodies[idx].wz);
    infoPlanet = -1;
}

static void focusOnDeepGalaxy(int idx)
{
    if(idx < 0 || idx >= N_DEEP_GALAXIES) return;
    cam.focusPlanet = FOCUS_DEEPGALAXY_BASE - idx;
    cam.mode = CAM_FREE;
    startCameraTransition(deepGalaxies[idx].scale * 2.8f + 24.0f, 36.0f, 12.0f, deepGalaxies[idx].x, deepGalaxies[idx].y, deepGalaxies[idx].z);
    infoPlanet = -1;
}

static void focusOnShip()
{
    cam.focusPlanet = FOCUS_SHIP;
    cam.mode = CAM_FREE;
    startCameraTransition(4.8f, 40.0f, 18.0f, ship.x, ship.y, ship.z);
    infoPlanet = -1;
}

static void focusOnComet()
{
    cam.focusPlanet = FOCUS_COMET;
    cam.mode = CAM_FREE;
    startCameraTransition(3.8f, 35.0f, 18.0f, comet.x, comet.y, comet.z);
    infoPlanet = -1;
}

static void focusExploreIndex(int idx)
{
    int total = totalExploreTargets();
    if(total <= 0) return;
    while(idx < 0) idx += total;
    idx %= total;
    currentExploreIndex = idx;
    if(idx < N_NEARBY_SYSTEMS) { focusOnNearbySystem(idx); return; }
    idx -= N_NEARBY_SYSTEMS;
    if(idx < N_OUTER_BODIES) { focusOnOuterBody(idx); return; }
    idx -= N_OUTER_BODIES;
    if(idx == 0) { focusOnShip(); return; }
    if(idx == 1) { focusOnComet(); return; }
    idx -= 2;
    focusOnDeepGalaxy(idx % 4);
}

static void cycleExploreTarget(int step)
{
    focusExploreIndex(currentExploreIndex + step);
}

static void focusSolarTarget(int keyIndex)
{
    if(keyIndex <= 0) {
        focusOnPlanet(FOCUS_SUN);
        return;
    }
    int planetIdx = keyIndex - 1;
    if(planetIdx >= 0 && planetIdx < NP) {
        focusOnPlanet(planetIdx);
        infoPlanet = planetIdx;
        showInfo = true;
    }
}

static const char* solarKeyLegendLabel(int slot)
{
    if(slot == 0) return "Sun";
    if(slot > 0 && slot <= NP) return planets[slot-1].name;
    return "";
}

static const char* nearbyFocusLegendLabel(int idx)
{
    static const char* shortNames[N_NEARBY_SYSTEMS] = {
        "Proxima", "A Cen A", "A Cen B", "Barnard", "Sirius", "TRAPPIST-1", "Kepler"
    };
    if(idx < 0 || idx >= N_NEARBY_SYSTEMS) return "";
    return shortNames[idx];
}

static const char* armCompactName(int arm)
{
    static const char* shortArms[N_GALAXY_ARMS] = {
        "Perseus", "Outer", "Cygnus", "Orion", "Carina", "Scutum", "Norma", "Swan"
    };
    if(arm < 0 || arm >= N_GALAXY_ARMS) return "";
    return shortArms[arm];
}

static void rect2D(float x,float y,float w,float h,float r,float g,float b,float a);
static void border2D(float x,float y,float w,float h,float r,float g,float b,float a);
static void text2D(float x,float y,const char* s,float r,float g,float b,void* font);
static int bitmapTextWidth(const char* s, void* font);

static void drawSelectorChip(float x,float y,float w,float h,const char* label,bool selected,float r,float g,float b)
{
    float br = selected ? r : 0.08f;
    float bg = selected ? g : 0.10f;
    float bb = selected ? b : 0.16f;
    rect2D(x, y, w, h, br, bg, bb, selected ? 0.94f : 0.82f);
    border2D(x, y, w, h, selected ? 1.0f : 0.38f, selected ? 0.92f : 0.58f, selected ? 0.72f : 0.92f, 0.84f);
    int tw = bitmapTextWidth(label, GLUT_BITMAP_HELVETICA_10);
    float tx = x + std::max(6.0f, (w - (float)tw) * 0.5f);
    text2D(tx, y + h*0.5f - 3, label,
           selected ? 1.0f : 0.84f,
           selected ? 0.96f : 0.90f,
           selected ? 0.76f : 1.0f,
           GLUT_BITMAP_HELVETICA_10);
}

static void initShip()
{
    ship.pathAngle = 0;
    ship.spd = 0.4f;
    ship.x = ship.y = ship.z = 0;
    ship.yaw = 0;
    ship.pitch = 0;
    ship.tlen = 0;
    ship.active = true;
    ship.batteryLevel = 100;
    ship.solarEfficiency = 100;
    ship.shieldLevel = 100;
    ship.engineThrust = 0.65f;
    ship.missionDay = 0;
    ship.bankAngle = 0.0f;
    ship.radarAngle = 0.0f;
    ship.engineHeat = 0.45f;
    ship.commSignal = 0.90f;
    ship.scanPhase = 0.0f;
}

static void initComet()
{
    comet.ang = 0;
    comet.spd = 0.8f;
    comet.orbitR = 38;
    comet.ecc = 0.82f;
    comet.incl = 162.0f;
    comet.x = comet.y = comet.z = 0;
    comet.np = COMET_PARTS;
    comet.active = true;
    comet.tailLength = 8.0f;
    comet.name = "Halley's Comet";
    for(int i=0;i<COMET_PARTS;i++) {
        comet.p[i].x = comet.p[i].y = comet.p[i].z = 0;
        comet.p[i].vx = comet.p[i].vy = comet.p[i].vz = 0;
        comet.p[i].life = 0;
        comet.p[i].sz = frandRange(0.015f, 0.065f);
        comet.p[i].r = frandRange(0.5f, 1.0f);
        comet.p[i].g = frandRange(0.7f, 1.0f);
        comet.p[i].b = 1.0f;
    }
}

static void initBlackHole()
{
    blackHole.x = 0;
    blackHole.y = 0;
    blackHole.z = 0;
    blackHole.size = 5;
    blackHole.diskRadius = 55;
    blackHole.rotation = 0;
    blackHole.jetHeight = 300;
    blackHole.lensRing = 8.5f;
}

static void initSpace()
{
    std::srand(42);
    generateSpiralGalaxy();
    initNebulae();
    initMeteors();
    initLensFlare();
    initAsteroids();
    initDeepSpace();
    initNearbySystems();
    initOuterBodies();
    initShip();
    initComet();
    initBlackHole();
    for(int i=0;i<NP;i++) {
        planets[i].tlen = 0;
        planets[i].wx = planets[i].wy = planets[i].wz = 0;
    }
}

static void initGL()
{
    glClearColor(0, 0, 0.004f, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat la[] = {0.012f,0.012f,0.022f,1};
    GLfloat ld[] = {1.0f,0.97f,0.88f,1};
    GLfloat ls[] = {1.0f,1.0f,0.95f,1};
    glLightfv(GL_LIGHT0, GL_AMBIENT,  la);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  ld);
    glLightfv(GL_LIGHT0, GL_SPECULAR, ls);

    GLfloat ms[] = {0.25f,0.25f,0.25f,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ms);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20);

    Q = gluNewQuadric();
    gluQuadricNormals(Q, GLU_SMOOTH);

    initPlanetData();
    initMoonData();
    initSpace();

    float armBase = (solarSystemArm / (float)N_GALAXY_ARMS) * TWO_PI;
    sunGalacticAngle = armBase + 0.55f * std::log(sunGalacticR + 1.0f);
    sunX = std::cos(sunGalacticAngle) * sunGalacticR;
    sunZ = std::sin(sunGalacticAngle) * sunGalacticR;
    sunY = 0;
    for(int i=0;i<N_NEARBY_SYSTEMS;i++){ nearbySystems[i].wx = sunX + nearbySystems[i].dx; nearbySystems[i].wy = sunY + nearbySystems[i].dy; nearbySystems[i].wz = sunZ + nearbySystems[i].dz; }
    for(int i=0;i<N_OUTER_BODIES;i++){ outerBodies[i].wx = sunX + outerBodies[i].orbitRadius; outerBodies[i].wy = sunY; outerBodies[i].wz = sunZ; }

    cam.dist = 500;
    cam.yaw = 90;
    cam.pitch = 75;
    cam.targetX = cam.targetY = cam.targetZ = 0;
    cam.smoothX = cam.smoothY = cam.smoothZ = 0;
    cam.mode = CAM_FREE;
    cam.focusPlanet = -1;
    cam.transitioning = false;
    cam.transT = 0;
    cam.orbitAngle = 0;
    cam.cinT = 0;
    cam.cinDuration = 1;
}

static void startCameraTransition(float nd,float ny,float np2,float ntx,float nty,float ntz)
{
    cam.transitioning = true;
    cam.transT = 0;
    cam.fromDist = cam.dist; cam.toDist = nd;
    cam.fromYaw = cam.yaw;   cam.toYaw = ny;
    cam.fromPitch = cam.pitch; cam.toPitch = np2;
    cam.fromTX = cam.targetX; cam.fromTY = cam.targetY; cam.fromTZ = cam.targetZ;
    cam.toTX = ntx; cam.toTY = nty; cam.toTZ = ntz;
}

static void focusOnPlanet(int idx)
{
    if(idx == FOCUS_BLACKHOLE) {
        cam.focusPlanet = FOCUS_BLACKHOLE;
        cam.mode = CAM_FREE;
        startCameraTransition(160, 90, 72, 0, 0, 0);
        infoPlanet = -1;
        return;
    }
    if(idx == FOCUS_SUN) {
        cam.focusPlanet = FOCUS_SUN;
        cam.mode = CAM_FREE;
        startCameraTransition(24, 30, 16, sunX, sunY, sunZ);
        infoPlanet = -1;
        return;
    }
    if(idx < 0 || idx >= NP) return;

    cam.focusPlanet = idx;
    cam.mode = CAM_FOLLOW;
    infoPlanet = idx;
    float d = planets[idx].size * 9 + 2;
    if(idx == JIDX || idx == SIDX) d += 4;
    if(idx == EIDX) d = 5.5f;
    if(idx == PIDX) d = 4.0f;
    startCameraTransition(d, 35, 18, planets[idx].wx, planets[idx].wy, planets[idx].wz);
    showInfo = true;
    showFocusPanel = true;
}

static void updateCamera()
{
    if(cam.transitioning) {
        cam.transT += 0.022f;
        if(cam.transT >= 1.0f) {
            cam.transT = 1.0f;
            cam.transitioning = false;
        }
        float t = smootherstepf(cam.transT);
        cam.dist = LERP(cam.fromDist, cam.toDist, t);
        cam.yaw = LERP(cam.fromYaw, cam.toYaw, t);
        cam.pitch = LERP(cam.fromPitch, cam.toPitch, t);
        cam.targetX = LERP(cam.fromTX, cam.toTX, t);
        cam.targetY = LERP(cam.fromTY, cam.toTY, t);
        cam.targetZ = LERP(cam.fromTZ, cam.toTZ, t);
    }

    if(!cam.transitioning) {
        if(cam.focusPlanet >= 0) {
            if(cam.mode == CAM_FOLLOW || cam.mode == CAM_ORBIT) {
                cam.targetX = planets[cam.focusPlanet].wx;
                cam.targetY = planets[cam.focusPlanet].wy;
                cam.targetZ = planets[cam.focusPlanet].wz;
            }
            if(cam.mode == CAM_ORBIT) cam.orbitAngle += 0.45f * simSpeed;
        } else if(cam.focusPlanet == FOCUS_SUN) {
            cam.targetX = sunX; cam.targetY = sunY; cam.targetZ = sunZ;
        } else if(cam.focusPlanet == FOCUS_BLACKHOLE) {
            cam.targetX = 0; cam.targetY = 0; cam.targetZ = 0;
        } else if(focusIsNearby()) {
            int idx = focusNearbyIndex();
            if(idx >= 0 && idx < N_NEARBY_SYSTEMS) {
                cam.targetX = nearbySystems[idx].wx;
                cam.targetY = nearbySystems[idx].wy;
                cam.targetZ = nearbySystems[idx].wz;
            }
        } else if(focusIsOuterBody()) {
            int idx = focusOuterIndex();
            if(idx >= 0 && idx < N_OUTER_BODIES) {
                cam.targetX = outerBodies[idx].wx;
                cam.targetY = outerBodies[idx].wy;
                cam.targetZ = outerBodies[idx].wz;
            }
        } else if(focusIsDeepGalaxy()) {
            int idx = focusDeepGalaxyIndex();
            if(idx >= 0 && idx < N_DEEP_GALAXIES) {
                cam.targetX = deepGalaxies[idx].x;
                cam.targetY = deepGalaxies[idx].y;
                cam.targetZ = deepGalaxies[idx].z;
            }
        } else if(focusIsShip()) {
            cam.targetX = ship.x; cam.targetY = ship.y; cam.targetZ = ship.z;
        } else if(focusIsComet()) {
            cam.targetX = comet.x; cam.targetY = comet.y; cam.targetZ = comet.z;
        }
    }

    cam.smoothX = LERP(cam.smoothX, cam.targetX, 0.06f);
    cam.smoothY = LERP(cam.smoothY, cam.targetY, 0.06f);
    cam.smoothZ = LERP(cam.smoothZ, cam.targetZ, 0.06f);
}


static void applyCamera()
{
    float useYaw = cam.yaw;
    if(cam.mode == CAM_ORBIT && cam.focusPlanet >= 0) useYaw = cam.orbitAngle;
    float ex = cam.smoothX + cam.dist * std::sin(DEG2RAD(useYaw)) * std::cos(DEG2RAD(cam.pitch));
    float ey = cam.smoothY + cam.dist * std::sin(DEG2RAD(cam.pitch));
    float ez = cam.smoothZ + cam.dist * std::cos(DEG2RAD(useYaw)) * std::cos(DEG2RAD(cam.pitch));
    gluLookAt(ex, ey, ez, cam.smoothX, cam.smoothY, cam.smoothZ, 0, 1, 0);
}

static void pushTrail(Planet& p)
{
    if(p.tlen < TRAIL_LEN) {
        p.tx[p.tlen] = p.wx; p.ty[p.tlen] = p.wy; p.tz[p.tlen] = p.wz; p.tlen++;
    } else {
        for(int i=0;i<TRAIL_LEN-1;i++) {
            p.tx[i] = p.tx[i+1]; p.ty[i] = p.ty[i+1]; p.tz[i] = p.tz[i+1];
        }
        p.tx[TRAIL_LEN-1] = p.wx; p.ty[TRAIL_LEN-1] = p.wy; p.tz[TRAIL_LEN-1] = p.wz;
    }
}

static void pushShipTrail()
{
    if(!ship.active) return;
    if(ship.tlen < TRAIL_LEN) {
        ship.tx[ship.tlen] = ship.x; ship.ty[ship.tlen] = ship.y; ship.tz[ship.tlen] = ship.z; ship.tlen++;
    } else {
        for(int i=0;i<TRAIL_LEN-1;i++) {
            ship.tx[i] = ship.tx[i+1]; ship.ty[i] = ship.ty[i+1]; ship.tz[i] = ship.tz[i+1];
        }
        ship.tx[TRAIL_LEN-1] = ship.x; ship.ty[TRAIL_LEN-1] = ship.y; ship.tz[TRAIL_LEN-1] = ship.z;
    }
}

static void updateCometParticles()
{
    if(!comet.active) return;
    for(int i=0;i<comet.np;i++) {
        comet.p[i].life -= 0.028f;
        if(comet.p[i].life <= 0) {
            comet.p[i].x = comet.x;
            comet.p[i].y = comet.y;
            comet.p[i].z = comet.z;
            comet.p[i].vx = frandRange(-0.012f, 0.012f) + (comet.x - sunX) * 0.018f;
            comet.p[i].vy = frandRange(-0.004f, 0.004f);
            comet.p[i].vz = frandRange(-0.012f, 0.012f) + (comet.z - sunZ) * 0.018f;
            comet.p[i].life = frandRange(0.45f, 1.0f);
        }
        comet.p[i].x += comet.p[i].vx;
        comet.p[i].y += comet.p[i].vy;
        comet.p[i].z += comet.p[i].vz;
    }
}

static void updateMeteors()
{
    if(!showMeteors) return;
    if(frand01() < 0.015f * simSpeed) spawnMeteor();
    for(int i=0;i<N_METEORS;i++) {
        if(!meteors[i].active) continue;
        meteors[i].life -= 0.016f * simSpeed;
        meteors[i].x += meteors[i].vx * 0.016f * simSpeed;
        meteors[i].y += meteors[i].vy * 0.016f * simSpeed;
        meteors[i].z += meteors[i].vz * 0.016f * simSpeed;
        if(meteors[i].life <= 0) meteors[i].active = false;
    }
}

static void updateUniverse()
{
    if(paused) return;

    sunTime += 0.016f * simSpeed;
    galacticTime += 0.00042f * simSpeed;

    float sOmega = galacticOmega(sunGalacticR);
    sunGalacticAngle += sOmega * 0.00042f * simSpeed;
    sunX = std::cos(sunGalacticAngle) * sunGalacticR;
    sunZ = std::sin(sunGalacticAngle) * sunGalacticR;
    sunY = 0;

    for(int i=0;i<NSTARS;i++) {
        if(galaxyStars[i].layer == 0 || galaxyStars[i].radius < 0.5f) continue;
        float omega = galacticOmega(galaxyStars[i].radius);
        float curA = galaxyStars[i].baseAngle + galacticTime * omega;
        galaxyStars[i].x = std::cos(curA) * galaxyStars[i].radius;
        galaxyStars[i].z = std::sin(curA) * galaxyStars[i].radius;
    }

    for(int i=0;i<NNEB;i++) {
        if(nebulae[i].radius < 0.5f) continue;
        float omega = galacticOmega(nebulae[i].radius);
        float curA = nebulae[i].baseAngle + galacticTime * omega;
        nebulae[i].wx = std::cos(curA) * nebulae[i].radius;
        nebulae[i].wz = std::sin(curA) * nebulae[i].radius;
        nebulae[i].rotation += 0.02f * simSpeed;
    }

    for(int i=0;i<N_SFR;i++) {
        if(sfRegions[i].radius < 0.5f) continue;
        float omega = galacticOmega(sfRegions[i].radius);
        float curA = sfRegions[i].baseAngle + galacticTime * omega;
        sfRegions[i].wx = std::cos(curA) * sfRegions[i].radius;
        sfRegions[i].wz = std::sin(curA) * sfRegions[i].radius;
    }

    for(int i=0;i<NP;i++) {
        planets[i].orbitAngle += planets[i].orbitSpeed * 0.06f * simSpeed;
        planets[i].spinAngle += planets[i].spinSpeed * 1.2f * simSpeed;
        float a = DEG2RAD(planets[i].orbitAngle);
        float pr = planets[i].orbitRadius * (1 - planets[i].eccentricity*planets[i].eccentricity) /
                   (1 + planets[i].eccentricity * std::cos(a));
        planets[i].wx = sunX + std::cos(a) * pr;
        planets[i].wy = sunY;
        planets[i].wz = sunZ + std::sin(a) * pr;
        if((frameCounter % 2) == 0) pushTrail(planets[i]);
    }

    for(int i=0;i<N_NEARBY_SYSTEMS;i++) {
        nearbySystems[i].wx = sunX + nearbySystems[i].dx;
        nearbySystems[i].wy = sunY + nearbySystems[i].dy;
        nearbySystems[i].wz = sunZ + nearbySystems[i].dz;
    }

    for(int i=0;i<N_OUTER_BODIES;i++) {
        outerBodies[i].orbitAngle += outerBodies[i].orbitSpeed * simSpeed;
        float a = DEG2RAD(outerBodies[i].orbitAngle);
        float inc = DEG2RAD(outerBodies[i].inclination);
        float pr = outerBodies[i].orbitRadius * (1 - outerBodies[i].eccentricity*outerBodies[i].eccentricity) /
                   (1 + outerBodies[i].eccentricity * std::cos(a));
        outerBodies[i].wx = sunX + std::cos(a) * pr;
        outerBodies[i].wy = sunY + std::sin(inc) * std::sin(a) * pr * 0.12f;
        outerBodies[i].wz = sunZ + std::sin(a) * pr * std::cos(inc);
    }

    earthMoon.ang += earthMoon.spd * 0.08f * simSpeed;
    earthMoon.spinAngle += earthMoon.spinSpeed * simSpeed;
    for(int m=0;m<4;m++) {
        jMoons[m].ang += jMoons[m].spd * 0.08f * simSpeed;
        jMoons[m].spinAngle += jMoons[m].spinSpeed * simSpeed;
    }
    titanMoon.ang += titanMoon.spd * 0.08f * simSpeed;
    for(int m=0;m<2;m++) marsMoons[m].ang += marsMoons[m].spd * 0.08f * simSpeed;
    for(int m=0;m<4;m++) uranusMoons[m].ang += uranusMoons[m].spd * 0.08f * simSpeed;
    neptuneMoon.ang += neptuneMoon.spd * 0.08f * simSpeed;
    plutoMoon.ang += plutoMoon.spd * 0.08f * simSpeed;

    for(int i=0;i<NAST;i++) {
        asteroids[i].ang += asteroids[i].spd * simSpeed;
        asteroids[i].rot += asteroids[i].rspd * simSpeed;
        if(asteroids[i].ang > 360) asteroids[i].ang -= 360;
        if(asteroids[i].rot > 360) asteroids[i].rot -= 360;
    }

    if(ship.active) {
        ship.pathAngle += ship.spd * simSpeed;
        ship.x = sunX + std::cos(DEG2RAD(ship.pathAngle)) * 14.0f;
        ship.z = sunZ + std::sin(DEG2RAD(ship.pathAngle)) * 14.0f;
        ship.y = sunY + 0.55f * std::sin(DEG2RAD(ship.pathAngle * 2));
        ship.yaw = ship.pathAngle + 90;
        ship.pitch = 25.0f * std::sin(DEG2RAD(ship.pathAngle * 2));
        if((frameCounter % 2) == 0) pushShipTrail();
        float d2s = DIST2(ship.x, ship.y, ship.z, sunX, sunY, sunZ);
        ship.solarEfficiency = CLAMP(100.0f * (5.5f / (d2s*d2s + 0.08f)), 0, 100);
        if(ship.solarEfficiency > 8) ship.batteryLevel += 0.05f * simSpeed;
        else ship.batteryLevel -= 0.10f * simSpeed;
        ship.batteryLevel = CLAMP(ship.batteryLevel, 0, 100);
        ship.missionDay = (int)(sunTime * 4);
        ship.bankAngle = 16.0f * std::sin(DEG2RAD(ship.pathAngle * 1.3f));
        ship.radarAngle += 1.3f * simSpeed;
        ship.engineHeat = LERP(ship.engineHeat, ship.engineThrust * (0.72f + 0.28f * std::sin(sunTime*4.0f)), 0.08f);
        ship.commSignal = 0.70f + 0.30f * CLAMP(ship.solarEfficiency / 100.0f, 0.0f, 1.0f);
        ship.scanPhase += 0.06f * simSpeed;
    }

    if(comet.active) {
        comet.ang += comet.spd * simSpeed;
        float a = DEG2RAD(comet.ang);
        float cr = comet.orbitR * (1 - comet.ecc*comet.ecc) / (1 + comet.ecc * std::cos(a));
        float inclR = DEG2RAD(comet.incl);
        comet.x = sunX + std::cos(a) * cr;
        comet.z = sunZ + std::sin(a) * cr * std::cos(inclR);
        comet.y = sunY + std::sin(a) * cr * std::sin(inclR) * 0.4f;
        updateCometParticles();
    }

    blackHole.rotation += 1.8f * simSpeed;
    updateMeteors();
    updateCamera();
}


static void text2D(float x,float y,const char* s,float r,float g,float b,void* font)
{
    float ui = nightMode ? 0.45f : 1.0f;
    glColor3f(r * ui, g * ui, b * ui);
    glRasterPos2f(x, y);
    for(const char* c=s; *c; ++c) glutBitmapCharacter(font, *c);
}

static void text2DShadow(float x,float y,const char* s,float r,float g,float b,void* font)
{
    glColor3f(0,0,0);
    glRasterPos2f(x+1, y-1);
    for(const char* c=s; *c; ++c) glutBitmapCharacter(font, *c);
    text2D(x, y, s, r, g, b, font);
}

static void rect2D(float x,float y,float w,float h,float r,float g,float b,float a)
{
    float ui = nightMode ? 0.45f : 1.0f;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r*ui, g*ui, b*ui, a);
    glBegin(GL_QUADS);
    glVertex2f(x, y); glVertex2f(x+w, y); glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
    glDisable(GL_BLEND);
}

static void border2D(float x,float y,float w,float h,float r,float g,float b,float a)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    glLineWidth(1.3f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y); glVertex2f(x+w, y); glVertex2f(x+w, y+h); glVertex2f(x, y+h);
    glEnd();
    glDisable(GL_BLEND);
}

static void drawBar2D(float x,float y,float w,float h,float fill,float r,float g,float b,const char* label)
{
    rect2D(x, y, w, h, 0.12f, 0.12f, 0.18f, 0.92f);
    rect2D(x+1, y+1, std::max(2.0f, (w-2)*fill), h-2, r, g, b, 1.0f);
    border2D(x, y, w, h, 0.5f, 0.5f, 0.6f, 0.75f);
    if(label) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%s %.0f%%", label, fill*100.0f);
        text2D(x+4, y+3, buf, 1, 1, 1, GLUT_BITMAP_HELVETICA_10);
    }
}

static void drawOrbit(float r,float rr,float gg,float bb,float aa,int segs=180)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(rr, gg, bb, aa);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<segs;i++) {
        float a = i * TWO_PI / segs;
        glVertex3f(sunX + std::cos(a) * r, sunY, sunZ + std::sin(a) * r);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawSelectedOrbit(int idx)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLineWidth(2.5f);
    glColor4f(planets[idx].r, planets[idx].g, planets[idx].b, 0.55f);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<240;i++) {
        float a = i * TWO_PI / 240.0f;
        glVertex3f(sunX + std::cos(a) * planets[idx].orbitRadius, sunY, sunZ + std::sin(a) * planets[idx].orbitRadius);
    }
    glEnd();
    glLineWidth(1.0f);
    glColor4f(planets[idx].r, planets[idx].g, planets[idx].b, 0.20f);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<240;i++) {
        float a = i * TWO_PI / 240.0f;
        float rr = planets[idx].orbitRadius + 0.15f;
        glVertex3f(sunX + std::cos(a) * rr, sunY, sunZ + std::sin(a) * rr);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static int bitmapTextWidth(const char* s, void* font)
{
    int w = 0;
    for(const char* c=s; *c; ++c) w += glutBitmapWidth(font, *c);
    return w;
}

static void drawFocusPulseAt(float wx,float wy,float wz,float radius,float r,float g,float b)
{
    glPushMatrix();
    glTranslatef(wx, wy, wz);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for(int ring=0; ring<3; ++ring) {
        float rr = radius * (1.45f + ring * 0.35f) * (1.0f + 0.06f * std::sin(sunTime * (3.0f + ring) + ring));
        float aa = 0.22f - ring * 0.06f;
        glColor4f(r, g, b, aa);
        glLineWidth(2.0f - ring * 0.35f);
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<56;i++) {
            float a = i * TWO_PI / 56.0f;
            glVertex3f(std::cos(a) * rr, 0.0f, std::sin(a) * rr);
        }
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

static void drawSelectionReticle(float size)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    float r = size * 1.9f;
    float pulse = 1.0f + 0.12f * std::sin(sunTime * 5.0f);
    glColor4f(1,1,1,0.55f);
    glLineWidth(2);
    glPushMatrix();
    glRotatef(sunTime*28,0,1,0);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<4;i++) {
        float ang = i * TWO_PI / 4.0f;
        for(float a=ang-0.45f; a<=ang+0.45f; a+=0.04f)
            glVertex3f(std::cos(a)*r*pulse, 0, std::sin(a)*r*pulse);
    }
    glEnd();
    glPopMatrix();

    glColor4f(1,1,0.5f,0.25f);
    glLineWidth(1);
    glPushMatrix();
    glRotatef(-sunTime*15,0,1,0);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<4;i++) {
        float ang = i*TWO_PI/4.0f + PI/4.0f;
        for(float a=ang-0.3f; a<=ang+0.3f; a+=0.04f)
            glVertex3f(std::cos(a)*r*pulse*1.08f,0,std::sin(a)*r*pulse*1.08f);
    }
    glEnd();
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

static void drawAtmosphere(float radius,float r,float g,float b,float a)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glColor4f(r,g,b,a*0.15f); gluSphere(Q, radius*1.18f, 20, 20);
    glColor4f(r,g,b,a*0.25f); gluSphere(Q, radius*1.10f, 22, 22);
    glColor4f(r,g,b,a);      gluSphere(Q, radius*1.05f, 24, 24);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawRingDisc(float iR,float oR,float r,float g,float b,float a,int segs=200)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glColor4f(r,g,b,a);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<=segs;i++) {
        float ang = i * TWO_PI / segs;
        glVertex3f(std::cos(ang)*oR, 0, std::sin(ang)*oR);
        glVertex3f(std::cos(ang)*iR, 0, std::sin(ang)*iR);
    }
    glEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawSaturnRings(float iR,float oR)
{
    struct Band { float i,o,r,g,b,a; } bands[] = {
        {0.00f,0.07f,0.42f,0.38f,0.28f,0.14f},
        {0.07f,0.25f,0.60f,0.54f,0.38f,0.44f},
        {0.25f,0.30f,0.18f,0.16f,0.12f,0.06f},
        {0.30f,0.52f,0.90f,0.82f,0.56f,0.68f},
        {0.52f,0.54f,0.10f,0.09f,0.07f,0.04f},
        {0.54f,0.75f,0.76f,0.68f,0.46f,0.50f},
        {0.75f,0.88f,0.55f,0.50f,0.34f,0.22f},
        {0.88f,1.00f,0.38f,0.34f,0.24f,0.10f}
    };
    float span = oR - iR;
    for(int i=0;i<8;i++) drawRingDisc(iR + span*bands[i].i, iR + span*bands[i].o, bands[i].r, bands[i].g, bands[i].b, bands[i].a);
}

static void drawUranusRing(float iR,float oR)
{
    drawRingDisc(iR, oR, 0.50f,0.68f,0.75f,0.22f);
    drawRingDisc(iR+0.04f, oR-0.06f, 0.42f,0.58f,0.68f,0.14f);
}

static void drawSunCorona()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for(int i=18;i>=1;i--) {
        float pulse = 1.0f + 0.065f * std::sin(sunTime*1.8f + i*0.7f);
        float inner = 1.0f + i*0.13f;
        float outer = inner + 0.10f*pulse;
        glColor4f(1.0f, 0.58f + i*0.015f, 0.05f, 0.020f*i);
        glBegin(GL_TRIANGLE_STRIP);
        for(int k=0;k<=130;k++) {
            float a = k * TWO_PI / 130.0f;
            float wave = 1.0f + 0.015f * std::sin(a*12 + sunTime*2.5f);
            glVertex3f(std::cos(a)*outer*wave, std::sin(a*2)*0.045f, std::sin(a)*outer*wave);
            glVertex3f(std::cos(a)*inner*wave, std::sin(a*2)*0.022f, std::sin(a)*inner*wave);
        }
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawSolarFlares()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for(int i=0;i<100;i++) {
        float a = i * TWO_PI / 100.0f + sunTime*0.22f;
        float len = 1.18f + 0.35f * std::sin(sunTime*4.5f + i*1.6f);
        float w = 0.018f + 0.014f * std::sin(sunTime*2.8f + i);
        float alpha = 0.22f + 0.08f * std::sin(sunTime*3 + i);
        glColor4f(1.0f, 0.70f + w, 0.16f, alpha);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex3f(std::cos(a)*1.02f, 0, std::sin(a)*1.02f);
        glVertex3f(std::cos(a)*len, std::sin(a*3)*w*1.5f, std::sin(a)*len);
        glEnd();
    }
    for(int p=0;p<6;p++) {
        float baseA = p*TWO_PI/6.0f + sunTime*0.08f;
        float archH = 1.5f + 0.4f * std::sin(sunTime*0.8f + p);
        glColor4f(1.0f,0.45f,0.08f,0.18f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_STRIP);
        for(int k=0;k<=20;k++) {
            float t = (float)k / 20.0f;
            float a = baseA + t*0.5f;
            float rr = 1.02f + archH*std::sin(t*PI)*0.5f;
            glVertex3f(std::cos(a)*rr, std::sin(t*PI)*archH*0.3f, std::sin(a)*rr);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawSun()
{
    glPushMatrix();
    glTranslatef(sunX,sunY,sunZ);
    drawSunCorona();
    GLfloat em[] = {0.92f,0.55f,0.12f,1};
    GLfloat z0[] = {0,0,0,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, em);
    glColor3f(1.0f,0.88f,0.20f);
    gluSphere(Q, 1.0f, 52, 52);
    drawSolarFlares();
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, z0);
    glPopMatrix();
}

static void drawEarthDecor(float sz)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.14f,0.55f,0.18f,0.90f);
    glPushMatrix(); glTranslatef(sz*0.55f,sz*0.25f,sz*0.15f); glScalef(0.30f,0.22f,0.15f); glutSolidSphere(sz,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*0.20f,sz*-0.28f,sz*0.08f); glScalef(0.14f,0.30f,0.12f); glutSolidSphere(sz,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*-0.14f,sz*0.08f,sz*-0.46f); glScalef(0.17f,0.38f,0.14f); glutSolidSphere(sz,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*-0.48f,sz*0.32f,sz*-0.32f); glScalef(0.36f,0.22f,0.22f); glutSolidSphere(sz,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*-0.62f,sz*-0.42f,sz*-0.18f); glScalef(0.15f,0.11f,0.10f); glutSolidSphere(sz,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*0.35f,sz*0.75f,sz*-0.10f); glScalef(0.08f,0.10f,0.06f); glutSolidSphere(sz,8,8); glPopMatrix();
    glColor4f(0.92f,0.96f,1.0f,0.90f);
    glPushMatrix(); glTranslatef(0,sz*0.93f,0); glutSolidSphere(sz*0.18f,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(0,sz*-0.93f,0); glutSolidSphere(sz*0.16f,10,10); glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0f,0.7f,0.2f,0.35f);
    glPointSize(2.0f);
    float cities[][3] = {
        {sz*0.50f,sz*0.28f,sz*0.20f},
        {sz*-0.10f,sz*0.25f,sz*-0.42f},
        {sz*-0.48f,sz*0.25f,sz*-0.30f},
        {sz*-0.52f,sz*0.05f,sz*-0.40f}
    };
    glBegin(GL_POINTS);
    for(int c=0;c<4;c++) glVertex3f(cities[c][0], cities[c][1], cities[c][2]);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    if(showClouds) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1,1,1,0.24f);
        glPushMatrix(); glRotatef(sunTime*17,0,1,0); gluSphere(Q,sz*1.04f,28,28); glPopMatrix();
        glColor4f(1,1,1,0.14f);
        glPushMatrix(); glRotatef(-sunTime*11,0.12f,1,0.04f); gluSphere(Q,sz*1.07f,22,22); glPopMatrix();
        glColor4f(1,1,1,0.06f);
        glPushMatrix(); glRotatef(sunTime*8,0.05f,1,0.08f); gluSphere(Q,sz*1.10f,18,18); glPopMatrix();
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
}

static void drawMarsDecor(float sz)
{
    glDisable(GL_LIGHTING);
    glColor4f(0.58f,0.26f,0.10f,0.55f);
    glPushMatrix(); glTranslatef(sz*0.3f,sz*0.1f,sz*0.2f); glScalef(0.21f,0.13f,0.16f); glutSolidSphere(sz,8,8); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*-0.4f,sz*-0.2f,sz*-0.3f); glScalef(0.19f,0.11f,0.13f); glutSolidSphere(sz,8,8); glPopMatrix();
    glColor4f(0.78f,0.48f,0.28f,0.55f);
    glPushMatrix(); glTranslatef(sz*-0.28f,sz*0.36f,sz*0.08f); glScalef(0.09f,0.07f,0.09f); glutSolidSphere(sz,8,8); glPopMatrix();
    glColor4f(0.94f,0.96f,0.98f,0.90f);
    glPushMatrix(); glTranslatef(0,sz*0.91f,0); glutSolidSphere(sz*0.17f,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(0,sz*-0.89f,0); glutSolidSphere(sz*0.13f,10,10); glPopMatrix();
    glEnable(GL_LIGHTING);
}

static void drawVenusDecor(float sz)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.95f,0.80f,0.50f,0.20f);
    glPushMatrix(); glRotatef(sunTime*-5,0,1,0); gluSphere(Q,sz*1.06f,24,24); glPopMatrix();
    glColor4f(0.90f,0.75f,0.44f,0.13f);
    glPushMatrix(); glRotatef(sunTime*-3,0.2f,1,0.1f); gluSphere(Q,sz*1.11f,20,20); glPopMatrix();
    glColor4f(0.85f,0.70f,0.38f,0.08f);
    glPushMatrix(); glRotatef(sunTime*-1.5f,0.1f,1,0.15f); gluSphere(Q,sz*1.16f,16,16); glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawMercuryDecor(float sz)
{
    glDisable(GL_LIGHTING);
    glColor4f(0.52f,0.48f,0.42f,0.62f);
    glPushMatrix(); glTranslatef(sz*0.30f,sz*0.20f,sz*0.10f); glScalef(0.13f,0.09f,0.11f); glutSolidSphere(sz,6,6); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*-0.20f,sz*-0.30f,sz*0.30f); glScalef(0.11f,0.08f,0.10f); glutSolidSphere(sz,6,6); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*0.10f,sz*-0.10f,sz*-0.40f); glScalef(0.16f,0.11f,0.13f); glutSolidSphere(sz,6,6); glPopMatrix();
    glPushMatrix(); glTranslatef(sz*-0.35f,sz*0.25f,sz*-0.18f); glScalef(0.09f,0.06f,0.08f); glutSolidSphere(sz,5,5); glPopMatrix();
    glEnable(GL_LIGHTING);
}

static void drawNeptuneDecor(float sz)
{
    glDisable(GL_LIGHTING);
    glColor4f(0.06f,0.12f,0.38f,0.68f);
    glPushMatrix(); glTranslatef(sz*0.78f,sz*0.12f,0); glScalef(0.19f,0.12f,0.13f); glutSolidSphere(sz,10,10); glPopMatrix();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for(int i=-4;i<=4;i++) {
        float y = i * 0.10f * sz;
        float rr = std::sqrt(CLAMP(sz*sz-y*y,0.0f,sz*sz));
        if(rr < 0.001f) continue;
        float brightness = 0.25f + 0.10f * std::abs(std::sin((float)i));
        glColor4f(0.18f,0.30f,brightness+0.4f,0.28f);
        glPushMatrix(); glTranslatef(0,y,0); glRotatef(90,1,0,0); glutSolidTorus(sz*0.009f,rr*0.98f,8,45); glPopMatrix();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawJupiterBands(float sz)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float bandColors[][4] = {
        {0.88f,0.76f,0.52f,0.72f}, {0.72f,0.56f,0.38f,0.72f},
        {0.90f,0.78f,0.55f,0.65f}, {0.68f,0.52f,0.35f,0.72f},
        {0.85f,0.72f,0.50f,0.65f}, {0.75f,0.60f,0.40f,0.72f},
        {0.88f,0.76f,0.52f,0.65f}, {0.70f,0.55f,0.36f,0.72f}
    };
    for(int i=-7;i<=7;i++) {
        float y = i * 0.07f * sz;
        float rr = std::sqrt(CLAMP(sz*sz-y*y,0.0f,sz*sz));
        int ci = (i+7) % 8;
        glColor4f(bandColors[ci][0], bandColors[ci][1], bandColors[ci][2], bandColors[ci][3]);
        glPushMatrix(); glTranslatef(0,y,0); glRotatef(90,1,0,0); glutSolidTorus(sz*0.010f, rr*0.98f, 10, 55); glPopMatrix();
    }
    glColor4f(0.78f,0.28f,0.12f,0.92f);
    glPushMatrix(); glTranslatef(sz*0.82f,sz*-0.09f,0); glScalef(0.24f,0.11f,0.15f); glutSolidSphere(sz,12,12); glPopMatrix();
    glColor4f(0.92f,0.42f,0.18f,0.65f);
    glPushMatrix(); glTranslatef(sz*0.82f,sz*-0.09f,0.02f); glScalef(0.16f,0.06f,0.09f); glutSolidSphere(sz,10,10); glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawPlanetBody(int idx)
{
    Planet& p = planets[idx];
    glColor3f(p.r, p.g, p.b);
    gluSphere(Q, p.size, 38, 38);
    if(idx == 0) drawMercuryDecor(p.size);
    if(idx == 1) drawVenusDecor(p.size);
    if(idx == EIDX) drawEarthDecor(p.size);
    if(idx == 3) drawMarsDecor(p.size);
    if(idx == JIDX) drawJupiterBands(p.size);
    if(idx == NIDX) drawNeptuneDecor(p.size);
    if(p.atmA > 0.001f) drawAtmosphere(p.size, p.atmR, p.atmG, p.atmB, p.atmA);
}

static void drawMoonObject(const Moon& m)
{
    glPushMatrix();
    glRotatef(m.spinAngle,0,1,0);
    glColor3f(m.r,m.g,m.b);
    gluSphere(Q, m.sz, 20, 20);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(m.r,m.g,m.b,0.10f);
    gluSphere(Q, m.sz*1.14f, 16, 16);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

static void drawMoonOrbit(float radius,float inclineDeg,float rr,float gg,float bb)
{
    float alpha = 0.10f + 0.08f * focusWeight(cam.targetX, cam.targetY, cam.targetZ);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(rr, gg, bb, alpha);
    glPushMatrix();
    glRotatef(inclineDeg,0,0,1);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<100;i++) {
        float a = i * TWO_PI / 100.0f;
        glVertex3f(std::cos(a)*radius, 0, std::sin(a)*radius);
    }
    glEnd();
    glPopMatrix();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawMoonAt(const Moon& m,const Planet& p)
{
    drawMoonOrbit(p.size + m.orb, m.incl, m.r, m.g, m.b);
    float a = DEG2RAD(m.ang);
    float r = p.size + m.orb;
    float inc = DEG2RAD(m.incl);
    glPushMatrix();
    glTranslatef(std::cos(a)*r, std::sin(inc)*0.10f, std::sin(a)*r);
    drawMoonObject(m);
    glPopMatrix();
}

static void drawPlanet(int idx)
{
    Planet& p = planets[idx];
    glPushMatrix();
    glTranslatef(p.wx,p.wy,p.wz);
    glRotatef(p.tilt,0,0,1);
    glRotatef(p.spinAngle,0,1,0);
    GLfloat ms[] = { p.spec/255.0f, p.spec/255.0f, p.spec/255.0f, 1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ms);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, p.spec);
    drawPlanetBody(idx);
    if(idx == cam.focusPlanet) drawSelectionReticle(p.size);
    if(idx == SIDX) {
        glPushMatrix(); glRotatef(90,1,0,0); drawSaturnRings(p.size*1.35f, p.size*2.25f); glPopMatrix();
    }
    if(idx == UIDX) {
        glPushMatrix(); glRotatef(98,0,0,1); glRotatef(90,1,0,0); drawUranusRing(p.size*1.24f, p.size*1.78f); glPopMatrix();
    }
    if(idx == EIDX) drawMoonAt(earthMoon, p);
    if(idx == 3) { drawMoonAt(marsMoons[0], p); drawMoonAt(marsMoons[1], p); }
    if(idx == JIDX) { for(int m=0;m<4;m++) drawMoonAt(jMoons[m], p); }
    if(idx == SIDX) drawMoonAt(titanMoon, p);
    if(idx == UIDX) { for(int m=0;m<4;m++) drawMoonAt(uranusMoons[m], p); }
    if(idx == NIDX) drawMoonAt(neptuneMoon, p);
    if(idx == PIDX) drawMoonAt(plutoMoon, p);
    glPopMatrix();
}

static void drawBlackHole()
{
    glPushMatrix();
    glTranslatef(blackHole.x, blackHole.y, blackHole.z);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float jetPulse = 0.75f + 0.25f * std::sin(sunTime * 9.5f);
    for(int side=0; side<2; side++) {
        glPushMatrix();
        glRotatef(side==0 ? 0.0f : 180.0f, 1,0,0);
        glRotatef(blackHole.rotation * 0.08f, 0,1,0);
        for(int L=0; L<5; L++) {
            float w = (1.0f + L*0.5f) * 4.5f;
            float alpha = (0.14f - L*0.022f) * jetPulse;
            glColor4f(0.35f+L*0.06f,0.55f+L*0.06f,1.0f,alpha);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0,0,0);
            for(int k=0;k<=40;k++) {
                float a = k*TWO_PI/40.0f;
                float h = blackHole.jetHeight * (1.0f - L*0.14f);
                float wave = 1.0f + 0.22f * std::sin(a*3 + sunTime*4.5f + L);
                glVertex3f(std::cos(a)*w*wave, h, std::sin(a)*w*wave);
            }
            glEnd();
        }
        glPopMatrix();
    }

    for(int L=0;L<8;L++) {
        float innerR = blackHole.size * (1.15f + L*0.50f);
        float outerR = innerR + blackHole.diskRadius*0.19f;
        float br = 1.0f - L*0.09f;
        float heatR = 1.0f * br;
        float heatG = (0.40f + L*0.08f) * br;
        float heatB = (0.04f + L*0.10f) * br;
        glColor4f(heatR,heatG,heatB,0.26f*br);
        glBegin(GL_TRIANGLE_STRIP);
        for(int k=0;k<=130;k++) {
            float a = k*TWO_PI/130.0f + DEG2RAD(blackHole.rotation)*(1.0f+L*0.22f);
            float wave = 1.0f + 0.055f*std::sin(a*9 + sunTime*4.5f + L);
            float wobble = (L - 4.0f)*0.18f;
            glVertex3f(std::cos(a)*outerR*wave,wobble,std::sin(a)*outerR*wave);
            glVertex3f(std::cos(a)*innerR*wave,wobble,std::sin(a)*innerR*wave);
        }
        glEnd();
    }

    glColor4f(1.0f,0.97f,0.88f,0.72f);
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<=90;i++) {
        float a = i*TWO_PI/90.0f - DEG2RAD(blackHole.rotation*2.2f);
        glVertex3f(std::cos(a)*blackHole.size*1.05f,0,std::sin(a)*blackHole.size*1.05f);
        glVertex3f(std::cos(a)*blackHole.size*2.6f,0,std::sin(a)*blackHole.size*2.6f);
    }
    glEnd();

    glColor4f(0.75f,0.55f,1.0f,0.09f);
    gluSphere(Q, blackHole.lensRing, 32, 32);

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(0,0,0);
    gluSphere(Q, blackHole.size, 36, 36);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.78f,0.55f,1.0f,0.20f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<=110;i++) {
        float a = i*TWO_PI/110.0f;
        float r = blackHole.size*1.12f + 0.32f*std::sin(a*4 + sunTime*2.8f);
        glVertex3f(std::cos(a)*r,0,std::sin(a)*r);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

static void drawGalaxyStars()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glPointSize(1.0f);
    glBegin(GL_POINTS);
    for(int i=0;i<NSTARS;i++) {
        int L = galaxyStars[i].layer;
        if(L == 1 || L == 2 || L == 3) continue;
        float tw = 0.85f + 0.15f * std::sin(sunTime*galaxyStars[i].twinkleSpeed*0.5f + galaxyStars[i].twinklePhase);
        float fade = depthFade(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z, 60.0f, 1200.0f);
        float fweight = focusWeight(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z);
        float b = galaxyStars[i].brightness * tw * (0.25f + 0.75f*fade) * fweight;
        float sr,sg,sb; getStarColor(galaxyStars[i].colorType,sr,sg,sb);
        if(L==4){sr*=0.38f;sg*=0.38f;sb*=0.38f;}
        if(L==5){sr*=0.68f;sg*=0.68f;sb*=0.78f;}
        if(L==6){sr*=0.55f;sg*=0.55f;sb*=0.65f;}
        glColor3f(sr*b, sg*b, sb*b);
        glVertex3f(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z);
    }
    glEnd();

    glPointSize(isCloseView() ? 1.2f : 1.6f);
    glBegin(GL_POINTS);
    for(int i=0;i<NSTARS;i++) {
        int L = galaxyStars[i].layer;
        if(L != 1 && L != 2) continue;
        float tw = 0.72f + 0.28f * std::sin(sunTime*galaxyStars[i].twinkleSpeed + galaxyStars[i].twinklePhase);
        float fade = depthFade(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z, 35.0f, 900.0f);
        float fweight = focusWeight(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z);
        float b = galaxyStars[i].brightness * tw * (0.25f + 0.75f*fade) * fweight;
        float sr,sg,sb; getStarColor(galaxyStars[i].colorType,sr,sg,sb);
        if(galaxyStars[i].armIndex >= 0 && L == 1) {
            float mix = 0.18f;
            sr = LERP(sr, ARM_COLORS[galaxyStars[i].armIndex][0], mix);
            sg = LERP(sg, ARM_COLORS[galaxyStars[i].armIndex][1], mix);
            sb = LERP(sb, ARM_COLORS[galaxyStars[i].armIndex][2], mix);
        }
        glColor3f(sr*b, sg*b, sb*b);
        glVertex3f(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z);
    }
    glEnd();

    if(showClusters) {
        glPointSize(isFarView() ? 2.0f : 2.6f);
        glBegin(GL_POINTS);
        for(int i=0;i<NSTARS;i++) {
            if(!galaxyStars[i].isCluster) continue;
            float tw = 0.68f + 0.32f * std::sin(sunTime*2.8f + galaxyStars[i].twinklePhase);
            float fade = depthFade(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z, 45.0f, 700.0f);
            float b = galaxyStars[i].brightness * tw * fade * focusWeight(galaxyStars[i].x,galaxyStars[i].y,galaxyStars[i].z);
            float sr,sg,sb; getStarColor(galaxyStars[i].colorType,sr,sg,sb);
            glColor3f(sr*b*1.35f, sg*b*1.15f, sb*b*0.85f);
            glVertex3f(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z);
        }
        glEnd();

        if(!isFarView()) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glPointSize(8.0f);
            glBegin(GL_POINTS);
            for(int i=0;i<NSTARS;i++) {
                if(!galaxyStars[i].isCluster || galaxyStars[i].brightness < 0.65f) continue;
                float tw = 0.55f + 0.45f * std::sin(sunTime*1.8f + galaxyStars[i].twinklePhase);
                float fade = depthFade(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z, 50.0f, 650.0f);
                glColor4f(1.0f, 0.88f, 0.45f, 0.08f * tw * fade);
                glVertex3f(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z);
            }
            glEnd();
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }

    if(!isFarView()) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        for(int i=0;i<NSTARS;i++) {
            if(galaxyStars[i].brightness < 0.86f || galaxyStars[i].layer == 0 || galaxyStars[i].layer == 4) continue;
            float tw = 0.72f + 0.28f * std::sin(sunTime*galaxyStars[i].twinkleSpeed + galaxyStars[i].twinklePhase);
            float fade = depthFade(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z, 60.0f, 700.0f);
            float b = galaxyStars[i].brightness * tw * fade;
            float sr,sg,sb; getStarColor(galaxyStars[i].colorType,sr,sg,sb);
            glColor4f(sr*b, sg*b, sb*b, 0.20f * fade);
            glVertex3f(galaxyStars[i].x, galaxyStars[i].y, galaxyStars[i].z);
        }
        glEnd();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

static void drawSFRs()
{
    if(!showSFR) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for(int i=0;i<N_SFR;i++) {
        float pulse = 0.7f + 0.3f * std::sin(sunTime*1.5f + sfRegions[i].phase);
        float fade = depthFade(sfRegions[i].wx,sfRegions[i].y,sfRegions[i].wz,50.0f,650.0f) * focusWeight(sfRegions[i].wx,sfRegions[i].y,sfRegions[i].wz);
        glColor4f(1.0f,0.3f,0.5f,0.025f*pulse*fade);
        glPushMatrix(); glTranslatef(sfRegions[i].wx,sfRegions[i].y,sfRegions[i].wz); glutSolidSphere(sfRegions[i].size*1.5f,8,8); glPopMatrix();
        glColor4f(1.0f,0.55f,0.65f,0.08f*pulse*fade);
        glPushMatrix(); glTranslatef(sfRegions[i].wx,sfRegions[i].y,sfRegions[i].wz); glutSolidSphere(sfRegions[i].size*0.6f,8,8); glPopMatrix();
        glColor4f(1.0f,0.8f,0.9f,0.18f*pulse*fade);
        glPushMatrix(); glTranslatef(sfRegions[i].wx,sfRegions[i].y,sfRegions[i].wz); glutSolidSphere(sfRegions[i].size*0.2f,6,6); glPopMatrix();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawNebulae()
{
    if(!showNebulae) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for(int i=0;i<NNEB;i++) {
        float pulse = 0.78f + 0.22f * std::sin(sunTime*0.35f + nebulae[i].phase);
        float fade = depthFade(nebulae[i].wx,nebulae[i].y,nebulae[i].wz,60.0f,700.0f) * focusWeight(nebulae[i].wx,nebulae[i].y,nebulae[i].wz);
        int layers = (nebulae[i].type == 2) ? 6 : 5;
        for(int L=0;L<layers;L++) {
            float sz = nebulae[i].size * (0.35f + 0.22f*L);
            float a = nebulae[i].a * (1.0f - 0.16f*L) * pulse * fade;
            glColor4f(nebulae[i].r, nebulae[i].g, nebulae[i].b, a);
            glPushMatrix();
            glTranslatef(nebulae[i].wx, nebulae[i].y, nebulae[i].wz);
            glRotatef(nebulae[i].rotation + L*28.0f, 0,1,0);
            glRotatef(L*14 + nebulae[i].phase*10, 1,0.3f,0);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0,0,0);
            int steps = (nebulae[i].type == 0) ? 42 : 36;
            for(int k=0;k<=steps;k++) {
                float ang = k*TWO_PI/steps;
                float jitter = 1.0f + 0.16f*std::sin(ang*3+i) + 0.08f*std::sin(ang*7 + i*0.5f);
                float yOff = std::sin(ang*0.5f) * sz * 0.28f;
                glVertex3f(std::cos(ang)*sz*jitter, yOff, std::sin(ang)*sz*jitter);
            }
            glEnd();
            if(nebulae[i].type == 2 && L == 2) {
                glColor4f(nebulae[i].r, nebulae[i].g, nebulae[i].b, a*2.5f);
                glLineWidth(1.5f);
                glBegin(GL_LINE_LOOP);
                for(int k=0;k<=60;k++) {
                    float ang = k*TWO_PI/60.0f;
                    glVertex3f(std::cos(ang)*sz*0.9f,0,std::sin(ang)*sz*0.9f);
                }
                glEnd();
            }
            glPopMatrix();
        }
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawAsteroids()
{
    if(!showAsteroidBelt) return;
    for(int i=0;i<NAST;i++) {
        float a = DEG2RAD(asteroids[i].ang);
        float x = std::cos(a) * asteroids[i].rad;
        float z = std::sin(a) * asteroids[i].rad;
        float worldX = sunX + x;
        float worldY = sunY + asteroids[i].ht;
        float worldZ = sunZ + z;
        float fade = depthFade(worldX,worldY,worldZ,15.0f,260.0f) * focusWeight(worldX,worldY,worldZ);
        if(fade < 0.08f) continue;
        glPushMatrix();
        glTranslatef(worldX, worldY, worldZ);
        glRotatef(asteroids[i].rot,0.7f,1.0f,0.3f);
        glScalef(asteroids[i].elong,1.0f,0.85f);
        float sz = asteroids[i].sz * (asteroids[i].isCeres ? 3.5f : 1.0f);
        glColor3f(asteroids[i].r*fade, asteroids[i].g*fade, asteroids[i].b*fade);
        glutSolidSphere(sz, asteroids[i].isCeres?12:5, asteroids[i].isCeres?12:5);
        glPopMatrix();
    }
}

static void drawComet()
{
    if(!showComet || !comet.active) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float dirX = comet.x - sunX, dirZ = comet.z - sunZ;
    float d = std::sqrt(dirX*dirX + dirZ*dirZ); if(d < 0.0001f) d = 1;
    dirX /= d; dirZ /= d;
    float fade = depthFade(comet.x,comet.y,comet.z,10.0f,500.0f) * focusWeight(comet.x,comet.y,comet.z);

    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=50;i++) {
        float t = (float)i / 50.0f;
        glColor4f(0.35f,0.58f,1.0f,(1-t)*0.45f*fade);
        glVertex3f(comet.x+dirX*t*comet.tailLength, comet.y+t*0.22f, comet.z+dirZ*t*comet.tailLength);
    }
    glEnd();

    glLineWidth(8.0f);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=30;i++) {
        float t = (float)i / 30.0f;
        glColor4f(0.35f,0.60f,1.0f,(1-t)*0.12f*fade);
        glVertex3f(comet.x+dirX*t*comet.tailLength*0.6f, comet.y+t*0.15f, comet.z+dirZ*t*comet.tailLength*0.6f);
    }
    glEnd();

    glLineWidth(3.5f);
    glBegin(GL_LINE_STRIP);
    for(int i=0;i<=40;i++) {
        float t = (float)i / 40.0f;
        float curve = t*t*2.5f;
        glColor4f(0.92f,0.82f,0.52f,(1-t)*0.35f*fade);
        glVertex3f(comet.x+dirX*t*comet.tailLength*0.72f-dirZ*curve,
                   comet.y+t*0.12f-t*0.08f,
                   comet.z+dirZ*t*comet.tailLength*0.72f+dirX*curve);
    }
    glEnd();

    glColor4f(0.65f,0.80f,1.0f,0.14f*fade);
    glPushMatrix(); glTranslatef(comet.x,comet.y,comet.z); glutSolidSphere(0.24f,12,12); glPopMatrix();
    glColor4f(0.80f,0.90f,1.0f,0.06f*fade);
    glPushMatrix(); glTranslatef(comet.x,comet.y,comet.z); glutSolidSphere(0.40f,10,10); glPopMatrix();

    glPointSize(2.2f);
    glBegin(GL_POINTS);
    for(int i=0;i<comet.np;i++) {
        float alpha = comet.p[i].life * 0.6f * fade;
        glColor4f(comet.p[i].r, comet.p[i].g, comet.p[i].b, alpha);
        glVertex3f(comet.p[i].x, comet.p[i].y, comet.p[i].z);
    }
    glEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    glPushMatrix();
    glTranslatef(comet.x,comet.y,comet.z);
    glColor3f(0.58f,0.55f,0.52f);
    glutSolidSphere(0.07f,16,16);
    glPopMatrix();
}

static void drawMeteors()
{
    if(!showMeteors) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for(int i=0;i<N_METEORS;i++) {
        if(!meteors[i].active) continue;
        float frac = meteors[i].life / meteors[i].maxLife;
        glPointSize(meteors[i].sz * 2.5f);
        glColor4f(1.0f,0.95f,0.80f,frac*0.9f);
        glBegin(GL_POINTS);
        glVertex3f(meteors[i].x, meteors[i].y, meteors[i].z);
        glEnd();
        float tailLen = 0.3f * (meteors[i].maxLife - meteors[i].life);
        float vx = meteors[i].vx, vy = meteors[i].vy, vz = meteors[i].vz;
        float speed = std::sqrt(vx*vx + vy*vy + vz*vz); if(speed < 0.001f) speed = 1;
        glLineWidth(meteors[i].sz);
        glBegin(GL_LINE_STRIP);
        for(int s=0;s<=8;s++) {
            float t = (float)s / 8.0f;
            float alpha = frac * (1-t) * 0.7f;
            glColor4f(1.0f,0.88f,0.65f,alpha);
            glVertex3f(meteors[i].x-vx/speed*tailLen*t, meteors[i].y-vy/speed*tailLen*t, meteors[i].z-vz/speed*tailLen*t);
        }
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawPlanetTrails()
{
    if(!showTrails) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    for(int i=0;i<NP;i++) {
        Planet& p = planets[i];
        if(p.tlen < 2) continue;
        float fadeFocus = focusWeight(p.wx,p.wy,p.wz);
        glLineWidth(4.0f);
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<p.tlen;j++) {
            float a = (float)j / p.tlen * 0.12f * fadeFocus;
            glColor4f(p.r,p.g,p.b,a);
            glVertex3f(p.tx[j],p.ty[j],p.tz[j]);
        }
        glEnd();
        glLineWidth(1.4f);
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<p.tlen;j++) {
            float a = (float)j / p.tlen * 0.72f * fadeFocus;
            glColor4f(p.r*0.85f,p.g*0.85f,p.b,a);
            glVertex3f(p.tx[j],p.ty[j],p.tz[j]);
        }
        glEnd();
    }
    if(showShip && ship.active && ship.tlen > 1) {
        glLineWidth(1.5f);
        glBegin(GL_LINE_STRIP);
        for(int i=0;i<ship.tlen;i++) {
            float a = (float)i / ship.tlen * 0.75f;
            glColor4f(0.30f,0.88f,1.0f,a);
            glVertex3f(ship.tx[i],ship.ty[i],ship.tz[i]);
        }
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawShip()
{
    if(!showShip || !ship.active) return;
    float fade = depthFade(ship.x,ship.y,ship.z,8.0f,300.0f) * focusWeight(ship.x,ship.y,ship.z);
    if(fade < 0.05f) return;

    glPushMatrix();
    glTranslatef(ship.x,ship.y,ship.z);
    glRotatef(ship.yaw,0,1,0);
    glRotatef(ship.pitch,1,0,0);
    glRotatef(ship.bankAngle,0,0,1);

    glColor3f(0.72f*fade,0.78f*fade,0.86f*fade);
    glPushMatrix();
    glScalef(1.8f,0.9f,0.9f);
    glutSolidSphere(0.055f,14,14);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.11f,0.02f,0.0f);
    glScalef(0.9f,0.55f,0.55f);
    glColor3f(0.84f*fade,0.90f*fade,0.98f*fade);
    glutSolidSphere(0.038f,12,12);
    glPopMatrix();

    glColor3f(0.62f*fade,0.68f*fade,0.78f*fade);
    glPushMatrix(); glTranslatef(-0.02f,0.0f,0.13f); glScalef(0.8f,0.6f,1.1f); glutSolidSphere(0.042f,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.02f,0.0f,-0.13f); glScalef(0.8f,0.6f,1.1f); glutSolidSphere(0.042f,10,10); glPopMatrix();

    glPushMatrix(); glTranslatef(0.01f,0.0f,0.0f); glRotatef(90,0,1,0); gluCylinder(Q,0.018f,0.016f,0.22f,10,4); glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.55f*fade,0.60f*fade,0.70f*fade);
    glLineWidth(1.4f);
    glBegin(GL_LINES);
    glVertex3f(-0.02f,0.0f,0.0f); glVertex3f(-0.02f,0.0f,0.13f);
    glVertex3f(-0.02f,0.0f,0.0f); glVertex3f(-0.02f,0.0f,-0.13f);
    glVertex3f(-0.02f,0.0f,0.13f); glVertex3f(-0.12f,0.0f,0.19f);
    glVertex3f(-0.02f,0.0f,-0.13f); glVertex3f(-0.12f,0.0f,-0.19f);
    glEnd();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.12f,0.26f,0.82f,0.92f*fade);
    glPushMatrix(); glTranslatef(-0.02f,0.0f,0.23f); glScalef(0.02f,0.20f,0.26f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.02f,0.0f,-0.23f); glScalef(0.02f,0.20f,0.26f); glutSolidCube(1.0f); glPopMatrix();
    glColor4f(0.30f,0.42f,0.98f,0.28f*fade);
    glPushMatrix(); glTranslatef(-0.01f,0.0f,0.23f); glScalef(0.03f,0.24f,0.30f); glutWireCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.01f,0.0f,-0.23f); glScalef(0.03f,0.24f,0.30f); glutWireCube(1.0f); glPopMatrix();

    glColor4f(0.86f,0.90f,0.96f,0.96f*fade);
    glPushMatrix();
    glTranslatef(0.04f,0.08f,0.0f);
    glRotatef(-25,0,0,1);
    glScalef(0.018f,0.10f,0.09f);
    glutSolidSphere(1.0f,10,10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.04f,0.08f,0.0f);
    glRotatef(ship.radarAngle,1,0,0);
    glutWireTorus(0.003f,0.05f,6,16);
    glPopMatrix();

    glColor4f(0.70f,0.76f,0.86f,0.80f*fade);
    glBegin(GL_TRIANGLES);
    glVertex3f(-0.06f, 0.03f, 0.06f); glVertex3f(-0.16f, 0.10f, 0.00f); glVertex3f(-0.06f,-0.01f,0.00f);
    glVertex3f(-0.06f, 0.03f,-0.06f); glVertex3f(-0.16f, 0.10f, 0.00f); glVertex3f(-0.06f,-0.01f,0.00f);
    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float ep = 0.65f + 0.35f * std::sin(sunTime*14.0f);
    float heat = CLAMP(ship.engineHeat,0.0f,1.0f);
    glColor4f(0.25f,0.55f,1.0f,0.58f*ep*fade);
    glPushMatrix(); glTranslatef(-0.18f,0.0f,0.11f); glutSolidSphere(0.028f,10,10); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.18f,0.0f,-0.11f); glutSolidSphere(0.028f,10,10); glPopMatrix();
    glColor4f(0.12f,0.35f,0.95f,0.26f*ep*fade);
    glPushMatrix(); glTranslatef(-0.24f,0.0f,0.11f); glutSolidSphere(0.050f,8,8); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.24f,0.0f,-0.11f); glutSolidSphere(0.050f,8,8); glPopMatrix();
    glLineWidth(2.0f);
    for(int side=-1; side<=1; side+=2) {
        glBegin(GL_LINE_STRIP);
        for(int e=0;e<14;e++) {
            float t = (float)e / 13.0f;
            float swirl = std::sin(t*PI*5.0f + sunTime*9.0f) * 0.010f * (1.0f-t);
            glColor4f(0.30f,0.65f,1.0f,0.45f*(1.0f-t)*heat*fade);
            glVertex3f(-0.18f - t*0.24f, swirl, side*0.11f + swirl*0.5f);
        }
        glEnd();
    }

    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glColor4f(1.0f,0.25f,0.25f,0.9f*fade); glVertex3f(0.12f,0.01f, 0.03f);
    glColor4f(0.25f,1.0f,0.45f,0.9f*fade); glVertex3f(0.12f,0.01f,-0.03f);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

static void drawLabel3D(float wx,float wy,float wz,const char* text,float r,float g,float b,void* font)
{
    GLdouble sx,sy,sz;
    if(gluProject(wx,wy,wz,gMV,gPR,gVP,&sx,&sy,&sz) != GL_TRUE) return;
    if(sz < 0 || sz > 1) return;
    float dx = wx - cam.smoothX;
    float dy = wy - cam.smoothY;
    float dz = wz - cam.smoothZ;
    float d = std::sqrt(dx*dx + dy*dy + dz*dz);
    bool focused = isWorldCurrentFocus(wx, wy, wz, 2.4f);
    float a = 1.0f - CLAMP((d - 20.0f) / 250.0f, 0.0f, 1.0f);
    if(focused) a = std::max(a, 0.98f);
    if(a < 0.08f) return;
    float px = (float)sx + 5.0f;
    float py = (float)sy + 4.0f;
    beginOrtho();
    if(focused) {
        int tw = bitmapTextWidth(text, font);
        rect2D(px - 8.0f, py - 5.0f, tw + 18.0f, 18.0f, r * 0.18f, g * 0.18f, b * 0.18f, 0.58f);
        border2D(px - 8.0f, py - 5.0f, tw + 18.0f, 18.0f, r, g, b, 0.78f);
        rect2D(px - 8.0f, py + 9.0f, tw + 18.0f, 2.0f, r * 0.55f, g * 0.55f, b * 0.55f, 0.85f);
        text2DShadow(px, py, text, std::min(1.0f, r * 1.12f), std::min(1.0f, g * 1.12f), std::min(1.0f, b * 1.12f), font);
        text2D(px + tw + 12.0f, py, "FOCUS", 1.0f, 0.96f, 0.74f, GLUT_BITMAP_HELVETICA_10);
    } else {
        text2DShadow(px, py, r * a > 0.02f ? text : text, r * a, g * a, b * a, font);
    }
    endOrtho();
}

static void drawAllLabels()
{
    if(!showLabels) return;
    glGetDoublev(GL_MODELVIEW_MATRIX,gMV);
    glGetDoublev(GL_PROJECTION_MATRIX,gPR);
    glGetIntegerv(GL_VIEWPORT,gVP);

    drawLabel3D(sunX,sunY+1.8f,sunZ,"Sun",1.0f,0.95f,0.20f,GLUT_BITMAP_HELVETICA_12);
    for(int i=0;i<NP;i++) {
        void* font = (cam.focusPlanet == i) ? GLUT_BITMAP_HELVETICA_18 : GLUT_BITMAP_HELVETICA_12;
        drawLabel3D(planets[i].wx, planets[i].wy+planets[i].size+0.38f, planets[i].wz, planets[i].name, planets[i].r, planets[i].g, planets[i].b, font);
    }
    for(int i=0;i<N_OUTER_BODIES;i++) {
        bool focused = focusIsOuterBody() && focusOuterIndex() == i;
        drawLabel3D(outerBodies[i].wx, outerBodies[i].wy+outerBodies[i].size+0.28f, outerBodies[i].wz, outerBodies[i].name, outerBodies[i].r, outerBodies[i].g, outerBodies[i].b, focused ? GLUT_BITMAP_HELVETICA_12 : GLUT_BITMAP_HELVETICA_10);
    }
    for(int i=0;i<N_NEARBY_SYSTEMS;i++) {
        bool focused = focusIsNearby() && focusNearbyIndex() == i;
        drawLabel3D(nearbySystems[i].wx, nearbySystems[i].wy+nearbySystems[i].size+0.45f, nearbySystems[i].wz, nearbySystems[i].name, nearbySystems[i].r, nearbySystems[i].g, nearbySystems[i].b, focused ? GLUT_BITMAP_HELVETICA_12 : GLUT_BITMAP_HELVETICA_10);
    }
    if(cam.focusPlanet == FOCUS_BLACKHOLE || cam.dist > 70.0f)
        drawLabel3D(0.0f,2.2f,0.0f,"Sagittarius A*",0.88f,0.68f,1.0f,GLUT_BITMAP_HELVETICA_18);
    if(showShip && ship.active) drawLabel3D(ship.x,ship.y+0.28f,ship.z,"Voyager-I",0.30f,0.92f,1.0f,GLUT_BITMAP_HELVETICA_10);
    if(showComet && comet.active) drawLabel3D(comet.x,comet.y+0.40f,comet.z,comet.name,0.78f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);

    if(focusIsDeepGalaxy()) {
        int idx = focusDeepGalaxyIndex();
        if(idx >= 0 && idx < N_DEEP_GALAXIES)
            drawLabel3D(deepGalaxies[idx].x, deepGalaxies[idx].y + deepGalaxies[idx].scale * 0.12f, deepGalaxies[idx].z,
                        deepGalaxies[idx].name, deepGalaxies[idx].r, deepGalaxies[idx].g, deepGalaxies[idx].b, GLUT_BITMAP_HELVETICA_12);
    }
}


static void drawArmLabels()
{
    if(!showArmNames || isCloseView()) return;
    glGetDoublev(GL_MODELVIEW_MATRIX,gMV);
    glGetDoublev(GL_PROJECTION_MATRIX,gPR);
    glGetIntegerv(GL_VIEWPORT,gVP);
    int activeArm = currentFocusArmIndex();
    for(int arm=0; arm<N_GALAXY_ARMS; arm++) {
        float armBase = (arm / (float)N_GALAXY_ARMS) * TWO_PI;
        float labelR = 148.0f + arm * 20.0f;
        float omega = galacticOmega(labelR);
        float curAngle = armBase + 0.55f * std::log(labelR+1.0f) + galacticTime*omega;
        float lx = std::cos(curAngle)*labelR;
        float lz = std::sin(curAngle)*labelR;
        GLdouble sx,sy,sz;
        if(gluProject(lx,0.0f,lz,gMV,gPR,gVP,&sx,&sy,&sz) != GL_TRUE) continue;
        if(sz < 0 || sz > 1) continue;
        bool active = (arm == activeArm);
        float fpx = (float)sx, fpy = (float)sy;
        beginOrtho();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(ARM_COLORS[arm][0],ARM_COLORS[arm][1],ARM_COLORS[arm][2], active ? 1.0f : 0.78f);
        glPointSize(active ? 7.0f : 4.0f);
        glBegin(GL_POINTS); glVertex2f(fpx,fpy); glEnd();
        if(active) {
            glColor4f(ARM_COLORS[arm][0],ARM_COLORS[arm][1],ARM_COLORS[arm][2],0.24f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(fpx, fpy);
            for(int k=0;k<=28;k++) {
                float a = k * TWO_PI / 28.0f;
                glVertex2f(fpx + std::cos(a) * 12.0f, fpy + std::sin(a) * 12.0f);
            }
            glEnd();
        }
        float labelOffX = (arm%2==0)?-74:74;
        float labelOffY = 16 + (arm*8)%40;
        if(showArmLines || active) {
            glColor4f(ARM_COLORS[arm][0],ARM_COLORS[arm][1],ARM_COLORS[arm][2],active ? 0.58f : 0.28f);
            glLineWidth(active ? 1.9f : 1.0f);
            glBegin(GL_LINES);
            glVertex2f(fpx,fpy);
            glVertex2f(fpx+labelOffX,fpy+labelOffY);
            glEnd();
        }
        if(active) {
            int tw = bitmapTextWidth(ARM_NAMES[arm], GLUT_BITMAP_HELVETICA_10);
            float tx = fpx + labelOffX - (arm%2==0 ? tw + 8.0f : 2.0f);
            float ty = fpy + labelOffY + 1.0f;
            rect2D(tx - 4.0f, ty - 4.0f, tw + 34.0f, 16.0f, ARM_COLORS[arm][0]*0.18f, ARM_COLORS[arm][1]*0.18f, ARM_COLORS[arm][2]*0.18f, 0.55f);
            border2D(tx - 4.0f, ty - 4.0f, tw + 34.0f, 16.0f, ARM_COLORS[arm][0], ARM_COLORS[arm][1], ARM_COLORS[arm][2], 0.68f);
            text2DShadow(tx, ty, ARM_NAMES[arm], 1.0f, 1.0f, 0.88f, GLUT_BITMAP_HELVETICA_10);
            text2D(tx + tw + 8.0f, ty, "FOCUS", 1.0f, 0.96f, 0.74f, GLUT_BITMAP_HELVETICA_10);
        } else {
            text2DShadow(fpx+labelOffX-(arm%2==0?70:2), fpy+labelOffY+3, ARM_NAMES[arm], ARM_COLORS[arm][0], ARM_COLORS[arm][1], ARM_COLORS[arm][2], GLUT_BITMAP_HELVETICA_10);
        }
        glDisable(GL_BLEND);
        endOrtho();
    }

    GLdouble sx,sy,sz;
    if(gluProject(sunX,sunY+3.5f,sunZ,gMV,gPR,gVP,&sx,&sy,&sz) != GL_TRUE) return;
    if(sz < 0 || sz > 1) return;
    float px = (float)sx, py = (float)sy;
    float pulse = 0.68f + 0.32f * std::sin(sunTime*3.2f);
    float pulse2 = 0.55f + 0.45f * std::sin(sunTime*2.0f+1.0f);
    beginOrtho();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for(int ring=0; ring<3; ring++) {
        float rr = 10.0f + ring*8.0f + (ring==0 ? 3.0f*pulse : ring==1 ? 4.0f*pulse2 : 2.0f*pulse);
        float alpha = 0.35f * (1.0f - ring*0.22f) * (ring==0 ? pulse : ring==1 ? pulse2 : pulse*pulse2);
        glColor4f(1.0f,1.0f,0.0f,alpha);
        glLineWidth(2.0f - ring*0.4f);
        glBegin(GL_LINE_LOOP);
        for(int k=0;k<=50;k++) {
            float a = k*TWO_PI/50.0f;
            glVertex2f(px + std::cos(a)*rr, py + std::sin(a)*rr);
        }
        glEnd();
    }
    glColor4f(1,1,0,0.94f);
    glLineWidth(2.2f);
    glBegin(GL_LINES);
    glVertex2f(px,py+18); glVertex2f(px,py+48);
    glEnd();
    glBegin(GL_TRIANGLES);
    glVertex2f(px,py+56); glVertex2f(px-6,py+44); glVertex2f(px+6,py+44);
    glEnd();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    text2DShadow(px-58,py+62,"YOU ARE HERE",1.0f,1.0f,0.0f,GLUT_BITMAP_HELVETICA_12);
    text2D(px-55,py+46,"Our Solar System",0.88f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
    text2D(px-53,py+32,"Orion Spur (Local Arm)",0.70f,0.80f,1.0f,GLUT_BITMAP_HELVETICA_10);
    if(cam.dist > 90) text2D(px-58,py+18,"26,000 ly from Galactic Centre",0.60f,0.70f,0.90f,GLUT_BITMAP_HELVETICA_10);
    glDisable(GL_BLEND);
    endOrtho();
}

static void drawGalaxyTitle()
{
    if(cam.dist < 110) return;
    glGetDoublev(GL_MODELVIEW_MATRIX,gMV);
    glGetDoublev(GL_PROJECTION_MATRIX,gPR);
    glGetIntegerv(GL_VIEWPORT,gVP);
    GLdouble sx,sy,sz;
    if(gluProject(0,6.0f,0,gMV,gPR,gVP,&sx,&sy,&sz) != GL_TRUE) return;
    if(sz < 0 || sz > 1) return;
    beginOrtho();
    float tx = (float)sx, ty = (float)sy;
    text2DShadow(tx-92,ty+16,"MILKY WAY GALAXY",0.96f,0.90f,1.0f,GLUT_BITMAP_HELVETICA_18);
    text2D(tx-62,ty-4,"Barred Spiral Galaxy - Top-Down View",0.62f,0.72f,0.92f,GLUT_BITMAP_HELVETICA_10);
    text2D(tx-52,ty-18,"Diameter: about 100,000 light-years",0.50f,0.60f,0.80f,GLUT_BITMAP_HELVETICA_10);
    text2D(tx-58,ty-32,"Stars: roughly 200-400 billion",0.45f,0.55f,0.75f,GLUT_BITMAP_HELVETICA_10);
    text2D(tx-50,ty-46,"Galactic Core: Sagittarius A*",0.78f,0.62f,1.0f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}

static void drawMiniMap()
{
    if(!showMap) return;
    float mx = WIN_W - 196.0f, my = 10.0f, mw = 186.0f, mh = 186.0f;
    float cx = mx + mw*0.5f, cy = my + mh*0.5f;
    float sc = mw*0.5f / 360.0f;
    beginOrtho();
    rect2D(mx-4,my-4,mw+8,mh+8,0.12f,0.22f,0.48f,0.50f);
    rect2D(mx-2,my-2,mw+4,mh+4,0.22f,0.36f,0.62f,0.95f);
    rect2D(mx,my,mw,mh,0.005f,0.020f,0.080f,0.95f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(1.0f);
    glBegin(GL_POINTS);
    for(int i=0;i<NSTARS;i+=8) {
        if(galaxyStars[i].layer == 0) continue;
        float px = cx + galaxyStars[i].x*sc;
        float py = cy + galaxyStars[i].z*sc;
        if(px<mx || px>mx+mw || py<my || py>my+mh) continue;
        float tw = 0.6f + 0.4f * std::sin(sunTime*galaxyStars[i].twinkleSpeed + galaxyStars[i].twinklePhase);
        float b = galaxyStars[i].brightness * tw * 0.7f;
        float sr,sg,sb; getStarColor(galaxyStars[i].colorType,sr,sg,sb);
        glColor3f(sr*b,sg*b,sb*b);
        glVertex2f(px,py);
    }
    glEnd();
    for(int arm=0; arm<N_GALAXY_ARMS; arm++) {
        float armBase = (arm/(float)N_GALAXY_ARMS) * TWO_PI;
        glColor4f(ARM_COLORS[arm][0]*0.5f,ARM_COLORS[arm][1]*0.5f,ARM_COLORS[arm][2]*0.5f,0.20f);
        glLineWidth(1.2f);
        glBegin(GL_LINE_STRIP);
        for(int j=0;j<=65;j++) {
            float t = (float)j / 65.0f;
            float r = 28.0f + t*455.0f;
            float omega = galacticOmega(r);
            float curA = armBase + 0.55f*std::log(r+1.0f) + galacticTime*omega;
            float px = cx + std::cos(curA)*r*sc;
            float py = cy + std::sin(curA)*r*sc;
            if(px<mx || px>mx+mw || py<my || py>my+mh) continue;
            glVertex2f(px,py);
        }
        glEnd();
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(1.0f,0.78f,0.35f,0.75f);
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx,cy);
    glEnd();
    glColor4f(1.0f,1.0f,0.0f,0.95f);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx + sunX*sc, cy + sunZ*sc);
    glEnd();
    text2DShadow(mx+10,my+mh-14,"Mini Galactic Map",0.90f,0.95f,1.0f,GLUT_BITMAP_HELVETICA_12);
    glDisable(GL_BLEND);
    endOrtho();
}

static void drawScaleAndFPS()
{
    if(!showScale) return;
    beginOrtho();
    rect2D(10,10,200,42,0.03f,0.05f,0.12f,0.75f);
    border2D(10,10,200,42,0.35f,0.45f,0.65f,0.65f);
    char buf[128];
    std::snprintf(buf,sizeof(buf),"FPS: %d   Sim Speed: %.2fx   Zoom: %.1f",fps,simSpeed,cam.dist);
    text2DShadow(18,34,buf,0.90f,0.96f,1.0f,GLUT_BITMAP_HELVETICA_12);
    std::snprintf(buf,sizeof(buf),"Target: %s", currentFocusLabel());
    text2D(18,18,buf,0.72f,0.82f,0.96f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}

static void drawInfoPanel()
{
    if(!showInfo) return;

    if(cam.focusPlanet == FOCUS_SUN) {
        beginOrtho();
        float x = 10, y = WIN_H - 254, w = 400, h = 244;
        rect2D(x,y,w,h,0.06f,0.05f,0.10f,0.90f);
        border2D(x,y,w,h,0.96f,0.76f,0.30f,0.84f);
        text2DShadow(x+14,y+h-22,"Sun",1.0f,0.92f,0.32f,GLUT_BITMAP_HELVETICA_18);
        text2D(x+14,y+h-40,"G-type main-sequence star and anchor of the Solar System.",0.96f,0.96f,0.92f,GLUT_BITMAP_HELVETICA_12);
        text2D(x+14,y+h-58,"Press 1-9 to jump directly from the Sun to any planet.",0.76f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+14,y+h-74,"This panel now switches correctly when 0 selects the solar center.",0.76f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
        char buf[192];
        std::snprintf(buf,sizeof(buf),"Galactic position (%.2f, %.2f, %.2f)",sunX,sunY,sunZ);
        text2D(x+14,y+h-100,buf,0.96f,0.90f,0.72f,GLUT_BITMAP_HELVETICA_12);
        std::snprintf(buf,sizeof(buf),"Simulation time %.2f   Galactic drift %.4f",sunTime,galacticTime);
        text2D(x+14,y+h-118,buf,0.88f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+14,y+h-136,"Neighborhood: Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, Pluto",0.88f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+14,y+h-154,"Visual overlays: orbital routes, zodiacal dust, beacon glows, and live target strip.",0.88f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+14,y+h-172,"Use the control panel strip on the right to confirm which numeric key selects which world.",0.86f,0.92f,0.96f,GLUT_BITMAP_HELVETICA_10);
        rect2D(x+w-118,y+h-30,100,18,0.24f,0.18f,0.06f,0.92f);
        border2D(x+w-118,y+h-30,100,18,1.0f,0.86f,0.42f,0.84f);
        text2D(x+w-90,y+h-17,"SOLAR CORE",1.0f,0.96f,0.72f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+14,y+14,"0 now always returns to the Sun cleanly.",0.86f,0.88f,0.96f,GLUT_BITMAP_HELVETICA_10);
        endOrtho();
        return;
    }

    int idx = -1;
    if(cam.focusPlanet >= 0 && cam.focusPlanet < NP) idx = cam.focusPlanet;
    else if(infoPlanet >= 0 && infoPlanet < NP) idx = infoPlanet;
    if(idx < 0 || idx >= NP) return;

    const Planet& p = planets[idx];
    float distToCam = DIST2(p.wx, p.wy, p.wz, cam.smoothX, cam.smoothY, cam.smoothZ);
    float distToSun = DIST2(p.wx, p.wy, p.wz, sunX, sunY, sunZ);
    float liveOrbitSpeed = p.orbitSpeed * simSpeed;

    beginOrtho();
    float x = 10, y = WIN_H - 254, w = 400, h = 244;
    rect2D(x,y,w,h,0.04f,0.06f,0.12f,0.90f);
    border2D(x,y,w,h,0.35f,0.50f,0.85f,0.82f);
    text2DShadow(x+14,y+h-22,p.name,p.r,p.g,p.b,GLUT_BITMAP_HELVETICA_18);
    text2D(x+14,y+h-40,p.fact,0.95f,0.95f,0.95f,GLUT_BITMAP_HELVETICA_12);
    text2D(x+14,y+h-58,p.detail,0.72f,0.82f,0.96f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+14,y+h-74,p.composition,0.70f,0.88f,0.70f,GLUT_BITMAP_HELVETICA_10);

    char buf[192];
    std::snprintf(buf,sizeof(buf),"Diameter: %.0f km    Gravity: %.2f m/s^2",p.realDiameter,p.gravity);
    text2D(x+14,y+h-100,buf,0.92f,0.88f,0.72f,GLUT_BITMAP_HELVETICA_12);
    std::snprintf(buf,sizeof(buf),"Distance from Sun: %.1f million km    Temp: %.0f C",p.realDistance,p.temperature);
    text2D(x+14,y+h-118,buf,0.88f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
    std::snprintf(buf,sizeof(buf),"Orbital Period: %.0f days    Day Length: %.1f hours",p.realOrbitalPeriod,p.realDayLength);
    text2D(x+14,y+h-136,buf,0.88f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
    std::snprintf(buf,sizeof(buf),"Moons: %d    Tilt: %.1f deg    Eccentricity: %.3f",p.moons,p.tilt,p.eccentricity);
    text2D(x+14,y+h-154,buf,0.88f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
    std::snprintf(buf,sizeof(buf),"Live sim distance to Sun: %.2f AU-ish units    Camera range: %.2f",distToSun,distToCam);
    text2D(x+14,y+h-172,buf,0.86f,0.92f,0.96f,GLUT_BITMAP_HELVETICA_10);
    std::snprintf(buf,sizeof(buf),"Spin %.2f    Orbit speed %.2f    Sim world pos (%.2f, %.2f, %.2f)",p.spinSpeed,liveOrbitSpeed,p.wx,p.wy,p.wz);
    text2D(x+14,y+h-190,buf,0.86f,0.92f,0.96f,GLUT_BITMAP_HELVETICA_10);

    const char* focusState = (cam.focusPlanet == idx) ? "FOCUSED" : "INFO ONLY";
    rect2D(x+w-118,y+h-30,100,18,(cam.focusPlanet == idx)?0.10f:0.18f,(cam.focusPlanet == idx)?0.35f:0.18f,0.12f,0.90f);
    border2D(x+w-118,y+h-30,100,18,0.85f,0.85f,0.45f,0.80f);
    text2D(x+w-92,y+h-17,focusState,1.0f,0.96f,0.72f,GLUT_BITMAP_HELVETICA_10);

    text2D(x+14,y+14,"Number-key focus now updates this panel immediately for the selected planet.",0.74f,0.84f,0.96f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}

static void drawPowerPanel()
{
    if(!showPowerPanel || !showShip || !ship.active) return;
    beginOrtho();
    float x = WIN_W - 270, y = WIN_H - 152, w = 258, h = 142;
    rect2D(x,y,w,h,0.04f,0.10f,0.14f,0.88f);
    border2D(x,y,w,h,0.22f,0.75f,1.0f,0.80f);
    text2DShadow(x+12,y+h-20,"Voyager Power HUD",0.35f,0.92f,1.0f,GLUT_BITMAP_HELVETICA_18);
    drawBar2D(x+12,y+h-46,w-24,16,ship.batteryLevel/100.0f,0.15f,0.85f,0.35f,"Battery");
    drawBar2D(x+12,y+h-72,w-24,16,ship.solarEfficiency/100.0f,0.25f,0.55f,1.0f,"Solar");
    drawBar2D(x+12,y+h-98,w-24,16,ship.shieldLevel/100.0f,0.95f,0.82f,0.22f,"Shield");
    char buf[64];
    std::snprintf(buf,sizeof(buf),"Mission Day: %d",ship.missionDay);
    text2D(x+14,y+18,buf,0.90f,0.95f,1.0f,GLUT_BITMAP_HELVETICA_12);
    std::snprintf(buf,sizeof(buf),"Thrust: %.0f%%",ship.engineThrust*100.0f);
    text2D(x+150,y+18,buf,0.85f,0.90f,1.0f,GLUT_BITMAP_HELVETICA_12);
    endOrtho();
}

static void drawHelpPanel()
{
    if(!showHelp) return;
    beginOrtho();

    float x = WIN_W - 506.0f;
    float y = 34.0f;
    float w = 490.0f;
    float h = 566.0f;
    rect2D(x,y,w,h,0.02f,0.05f,0.12f,0.92f);
    border2D(x,y,w,h,0.40f,0.52f,0.82f,0.84f);
    rect2D(x+2,y+h-52,w-4,40,0.06f,0.10f,0.20f,0.94f);
    border2D(x+2,y+h-52,w-4,40,0.28f,0.44f,0.78f,0.44f);

    float fr=0.86f, fg=0.90f, fb=1.0f;
    currentFocusColor(fr, fg, fb);
    int activeArm = currentFocusArmIndex();
    char buf[256];

    text2DShadow(x+16,y+h-24,"Control Panel",0.96f,0.98f,1.0f,GLUT_BITMAP_HELVETICA_18);
    text2D(x+18,y+h-42,"Improved focus dashboard for planets, stars, labels, and galactic arms",0.72f,0.86f,1.0f,GLUT_BITMAP_HELVETICA_10);

    float focusCardX = x + w - 176.0f;
    float focusCardY = y + h - 48.0f;
    rect2D(focusCardX, focusCardY-28.0f, 160.0f, 32.0f, fr*0.16f, fg*0.16f, fb*0.16f, 0.94f);
    border2D(focusCardX, focusCardY-28.0f, 160.0f, 32.0f, fr, fg, fb, 0.86f);
    text2D(focusCardX+10.0f, focusCardY-10.0f, currentFocusLabel(), 1.0f, 0.97f, 0.78f, GLUT_BITMAP_HELVETICA_12);
    text2D(focusCardX+10.0f, focusCardY-24.0f, currentFocusCategory(), 0.74f, 0.88f, 1.0f, GLUT_BITMAP_HELVETICA_10);

    float summaryY = y + h - 118.0f;
    rect2D(x+12, summaryY-4, w-24, 58, 0.04f,0.08f,0.16f,0.70f);
    border2D(x+12, summaryY-4, w-24, 58, 0.22f,0.34f,0.56f,0.46f);

    if(cam.focusPlanet >= 0 && cam.focusPlanet < NP) {
        std::snprintf(buf,sizeof(buf),"%s | gravity %.2f m/s^2 | temp %.0f C | moons %d",
                      planets[cam.focusPlanet].name, planets[cam.focusPlanet].gravity,
                      planets[cam.focusPlanet].temperature, planets[cam.focusPlanet].moons);
        text2D(x+20, summaryY+34, buf, 0.92f,0.94f,1.0f, GLUT_BITMAP_HELVETICA_12);
        std::snprintf(buf,sizeof(buf),"Orbit %.2f AU(sim) | day %.1f h | diameter %.0f km | arm %s",
                      planets[cam.focusPlanet].orbitRadius, planets[cam.focusPlanet].realDayLength,
                      planets[cam.focusPlanet].realDiameter, armCompactName(activeArm));
        text2D(x+20, summaryY+18, buf, 0.78f,0.88f,0.98f, GLUT_BITMAP_HELVETICA_10);
        std::snprintf(buf,sizeof(buf),"Composition: %s", planets[cam.focusPlanet].composition);
        text2D(x+20, summaryY+4, buf, 0.78f,0.90f,0.80f, GLUT_BITMAP_HELVETICA_10);
    } else if(cam.focusPlanet == FOCUS_SUN) {
        text2D(x+20, summaryY+34, "Sun | G-type main-sequence star | centre of the Solar System view", 0.98f,0.92f,0.70f, GLUT_BITMAP_HELVETICA_12);
        text2D(x+20, summaryY+18, "Use 0 for the Sun and 1 to 9 for Mercury through Pluto.", 0.78f,0.88f,0.98f, GLUT_BITMAP_HELVETICA_10);
        std::snprintf(buf,sizeof(buf),"Active arm solution: %s", armCompactName(activeArm));
        text2D(x+20, summaryY+4, buf, 0.78f,0.90f,0.80f, GLUT_BITMAP_HELVETICA_10);
    } else if(cam.focusPlanet == FOCUS_BLACKHOLE) {
        text2D(x+20, summaryY+34, "Sagittarius A* | supermassive black hole at the Milky Way core", 0.92f,0.82f,1.0f, GLUT_BITMAP_HELVETICA_12);
        text2D(x+20, summaryY+18, "Accretion disk, lensing shell, and bipolar jets are active.", 0.78f,0.88f,0.98f, GLUT_BITMAP_HELVETICA_10);
        text2D(x+20, summaryY+4, "Arm focus is suspended here because this is the galactic core.", 0.78f,0.90f,0.80f, GLUT_BITMAP_HELVETICA_10);
    } else if(focusIsNearby()) {
        int idx = focusNearbyIndex();
        if(idx >= 0 && idx < N_NEARBY_SYSTEMS) {
            std::snprintf(buf,sizeof(buf),"%s | %s | planets %d", nearbySystems[idx].name, nearbySystems[idx].classification, nearbySystems[idx].exoplanets);
            text2D(x+20, summaryY+34, buf, 0.96f,0.90f,0.78f, GLUT_BITMAP_HELVETICA_12);
            text2D(x+20, summaryY+18, nearbySystems[idx].note, 0.78f,0.88f,0.98f, GLUT_BITMAP_HELVETICA_10);
            std::snprintf(buf,sizeof(buf),"Offset from Sun: (%.0f, %.0f, %.0f) | arm %s", nearbySystems[idx].dx, nearbySystems[idx].dy, nearbySystems[idx].dz, armCompactName(activeArm));
            text2D(x+20, summaryY+4, buf, 0.78f,0.90f,0.80f, GLUT_BITMAP_HELVETICA_10);
        }
    } else if(focusIsOuterBody()) {
        int idx = focusOuterIndex();
        if(idx >= 0 && idx < N_OUTER_BODIES) {
            std::snprintf(buf,sizeof(buf),"%s | %s", outerBodies[idx].name, outerBodies[idx].kind);
            text2D(x+20, summaryY+34, buf, 0.88f,0.94f,1.0f, GLUT_BITMAP_HELVETICA_12);
            text2D(x+20, summaryY+18, outerBodies[idx].note, 0.78f,0.88f,0.98f, GLUT_BITMAP_HELVETICA_10);
            std::snprintf(buf,sizeof(buf),"Orbit %.1f | inclination %.0f deg | arm %s", outerBodies[idx].orbitRadius, outerBodies[idx].inclination, armCompactName(activeArm));
            text2D(x+20, summaryY+4, buf, 0.78f,0.90f,0.80f, GLUT_BITMAP_HELVETICA_10);
        }
    } else if(focusIsDeepGalaxy()) {
        int idx = focusDeepGalaxyIndex();
        if(idx >= 0 && idx < N_DEEP_GALAXIES) {
            std::snprintf(buf,sizeof(buf),"%s | deep-sky analog galaxy", deepGalaxies[idx].name);
            text2D(x+20, summaryY+34, buf, 0.90f,0.86f,1.0f, GLUT_BITMAP_HELVETICA_12);
            std::snprintf(buf,sizeof(buf),"Scale %.1f | position (%.0f, %.0f, %.0f)", deepGalaxies[idx].scale, deepGalaxies[idx].x, deepGalaxies[idx].y, deepGalaxies[idx].z);
            text2D(x+20, summaryY+18, buf, 0.78f,0.88f,0.98f, GLUT_BITMAP_HELVETICA_10);
            text2D(x+20, summaryY+4, "Focused galaxy labels now get stronger halo priority.", 0.78f,0.90f,0.80f, GLUT_BITMAP_HELVETICA_10);
        }
    } else if(focusIsShip()) {
        text2D(x+20, summaryY+34, "Voyager-I | exploration craft in heliocentric cruise", 0.40f,0.94f,1.0f, GLUT_BITMAP_HELVETICA_12);
        std::snprintf(buf,sizeof(buf),"Signal %.0f%% | heat %.0f%% | mission day %d | arm %s", ship.commSignal*100.0f, ship.engineHeat*100.0f, ship.missionDay, armCompactName(activeArm));
        text2D(x+20, summaryY+18, buf, 0.78f,0.88f,0.98f, GLUT_BITMAP_HELVETICA_10);
        text2D(x+20, summaryY+4, "Telemetry remains available in the compact ship panel at lower right.", 0.78f,0.90f,0.80f, GLUT_BITMAP_HELVETICA_10);
    } else if(focusIsComet()) {
        text2D(x+20, summaryY+34, "Halley's Comet | icy body with active ion and dust tails", 0.80f,0.90f,1.0f, GLUT_BITMAP_HELVETICA_12);
        text2D(x+20, summaryY+18, "Use Q to show or hide the comet and [ ] to cycle outward targets.", 0.78f,0.88f,0.98f, GLUT_BITMAP_HELVETICA_10);
        std::snprintf(buf,sizeof(buf),"Current arm solution: %s", armCompactName(activeArm));
        text2D(x+20, summaryY+4, buf, 0.78f,0.90f,0.80f, GLUT_BITMAP_HELVETICA_10);
    }

    float chipY = summaryY - 46.0f;
    text2D(x+12, chipY + 30.0f, "Display Status", 0.94f,0.96f,1.0f, GLUT_BITMAP_HELVETICA_12);
    drawSelectorChip(x+12,  chipY, 110, 22, showLabels ? "Labels Live" : "Labels Off", showLabels, 0.10f,0.24f,0.34f);
    drawSelectorChip(x+130, chipY, 116, 22, showArmNames ? "Arm Names" : "Names Off", showArmNames, 0.10f,0.24f,0.20f);
    drawSelectorChip(x+254, chipY, 104, 22, showArmLines ? "Arm Lines" : "Lines Off", showArmLines, 0.22f,0.20f,0.10f);
    drawSelectorChip(x+366, chipY, 108, 22, softFocusMode ? "Soft Focus" : "Hard Focus", softFocusMode, 0.16f,0.18f,0.28f);

    float solarTitleY = chipY - 52.0f;
    text2D(x+12, solarTitleY + 72.0f, "Solar Focus", 0.94f,0.96f,1.0f, GLUT_BITMAP_HELVETICA_12);
    text2D(x+86, solarTitleY + 72.0f, "0 to 9 keyboard targets", 0.68f,0.84f,1.0f, GLUT_BITMAP_HELVETICA_10);
    for(int slot=0; slot<=NP; ++slot) {
        int row = slot / 5;
        int col = slot % 5;
        float bx = x + 12.0f + col * 92.0f;
        float by = solarTitleY + 34.0f - row * 40.0f;
        bool selected = (slot == 0) ? (cam.focusPlanet == FOCUS_SUN) : (cam.focusPlanet == slot - 1);
        float baseR = selected ? fr * 0.22f : 0.08f;
        float baseG = selected ? fg * 0.22f : 0.10f;
        float baseB = selected ? fb * 0.22f : 0.16f;
        rect2D(bx, by, 82.0f, 30.0f, baseR, baseG, baseB, 0.92f);
        border2D(bx, by, 82.0f, 30.0f, selected ? fr : 0.42f, selected ? fg : 0.58f, selected ? fb : 0.92f, 0.82f);
        char keyBuf[8]; std::snprintf(keyBuf,sizeof(keyBuf),"%d",slot);
        text2D(bx + 8.0f, by + 17.0f, keyBuf, selected ? 1.0f : 0.82f, selected ? 0.96f : 0.88f, selected ? 0.72f : 1.0f, GLUT_BITMAP_HELVETICA_12);
        const char* label = solarKeyLegendLabel(slot);
        int tw = bitmapTextWidth(label, GLUT_BITMAP_HELVETICA_10);
        text2D(bx + std::max(8.0f, (82.0f - (float)tw) * 0.5f), by + 5.0f, label, selected ? 1.0f : 0.78f, selected ? 0.92f : 0.84f, selected ? 0.70f : 0.96f, GLUT_BITMAP_HELVETICA_10);
    }

    float starTitleY = solarTitleY - 104.0f;
    text2D(x+12, starTitleY + 62.0f, "Nearby Star Focus", 0.94f,0.96f,1.0f, GLUT_BITMAP_HELVETICA_12);
    text2D(x+130, starTitleY + 62.0f, "Cycle with [ and ]", 0.68f,0.84f,1.0f, GLUT_BITMAP_HELVETICA_10);
    for(int i=0;i<N_NEARBY_SYSTEMS;i++) {
        int row = i / 4;
        int col = i % 4;
        float bx = x + 12.0f + col * 116.0f;
        float by = starTitleY + 28.0f - row * 30.0f;
        bool selected = focusIsNearby() && focusNearbyIndex() == i;
        drawSelectorChip(bx, by, 106.0f, 22.0f, nearbyFocusLegendLabel(i), selected,
                         nearbySystems[i].r * 0.35f, nearbySystems[i].g * 0.35f, nearbySystems[i].b * 0.35f);
    }

    float groupY = starTitleY - 78.0f;
    text2D(x+12, groupY + 58.0f, "Focus Groups", 0.94f,0.96f,1.0f, GLUT_BITMAP_HELVETICA_12);
    drawSelectorChip(x+12,  groupY + 28.0f, 108, 22, "Sun",     cam.focusPlanet == FOCUS_SUN, 0.26f,0.18f,0.06f);
    drawSelectorChip(x+130, groupY + 28.0f, 108, 22, "Planets", cam.focusPlanet >= 0 && cam.focusPlanet < NP, 0.12f,0.25f,0.08f);
    drawSelectorChip(x+248, groupY + 28.0f, 108, 22, "Nearby",  focusIsNearby(), 0.22f,0.16f,0.08f);
    drawSelectorChip(x+366, groupY + 28.0f, 108, 22, "Outer",   focusIsOuterBody(), 0.18f,0.18f,0.10f);
    drawSelectorChip(x+12,  groupY + 2.0f, 108, 22, "Core",     cam.focusPlanet == FOCUS_BLACKHOLE, 0.20f,0.08f,0.30f);
    drawSelectorChip(x+130, groupY + 2.0f, 108, 22, "Deep",     focusIsDeepGalaxy(), 0.24f,0.10f,0.30f);
    drawSelectorChip(x+248, groupY + 2.0f, 108, 22, "Ship",     focusIsShip(), 0.06f,0.22f,0.28f);
    drawSelectorChip(x+366, groupY + 2.0f, 108, 22, "Comet",    focusIsComet(), 0.18f,0.22f,0.28f);

    float armY = groupY - 100.0f;
    text2D(x+12, armY + 92.0f, "Galactic Arms", 0.94f,0.96f,1.0f, GLUT_BITMAP_HELVETICA_12);
    std::snprintf(buf,sizeof(buf),"Active arm: %s", activeArm >= 0 ? ARM_NAMES[activeArm] : "No Arm Focus");
    rect2D(x+120.0f, armY + 78.0f, 196.0f, 20.0f, fr*0.10f, fg*0.10f, fb*0.10f, 0.86f);
    border2D(x+120.0f, armY + 78.0f, 196.0f, 20.0f, fr, fg, fb, 0.72f);
    text2D(x+130.0f, armY + 84.0f, buf, 0.84f,0.92f,1.0f, GLUT_BITMAP_HELVETICA_10);
    for(int arm=0; arm<N_GALAXY_ARMS; ++arm) {
        int row = arm / 2;
        int col = arm % 2;
        float bx = x + 12.0f + col * 232.0f;
        float by = armY + 54.0f - row * 20.0f;
        bool selected = (arm == activeArm);
        drawSelectorChip(bx, by, 220.0f, 16.0f, armCompactName(arm), selected,
                         ARM_COLORS[arm][0] * 0.35f, ARM_COLORS[arm][1] * 0.35f, ARM_COLORS[arm][2] * 0.35f);
    }
    text2D(x+12, armY - 22.0f, "Highlighted arm follows the current focus solution instead of staying fixed on Orion only.", 0.78f,0.88f,0.78f, GLUT_BITMAP_HELVETICA_10);

    float controlsY = y + 22.0f;
    rect2D(x+12.0f, controlsY, w-24.0f, 98.0f, 0.03f,0.07f,0.14f,0.76f);
    border2D(x+12.0f, controlsY, w-24.0f, 98.0f, 0.22f,0.34f,0.56f,0.42f);
    text2D(x+12.0f, controlsY + 84.0f, "Quick Controls", 0.94f,0.96f,1.0f, GLUT_BITMAP_HELVETICA_12);
    const char* leftCol[] = {
        "0 / 1-9  : focus Sun or planets",
        "B        : focus galactic core",
        "F / O    : follow or orbit mode",
        "P        : pause simulation",
        "+ / -    : simulation speed",
        "[ / ]    : cycle outward targets"
    };
    const char* rightCol[] = {
        "T / E    : trails or orbits",
        "L / S    : labels or scale HUD",
        "C / G    : clouds or grid",
        "N / X    : arm names or arm lines",
        "K / V / Z: clusters, nebulae, SFR",
        "J        : hide or show this panel"
    };
    for(int i=0;i<6;i++) {
        text2D(x+18.0f, controlsY + 66.0f - i*12.0f, leftCol[i], 0.82f,0.90f,1.0f, GLUT_BITMAP_HELVETICA_10);
        text2D(x+248.0f, controlsY + 66.0f - i*12.0f, rightCol[i], 0.82f,0.90f,1.0f, GLUT_BITMAP_HELVETICA_10);
    }

    endOrtho();
}

static void drawGalaxyDiagnosticsPanel()
{
    if(!showGalaxyDiagnostics) return;
    beginOrtho();
    float x = 390, y = WIN_H - 138, w = 340, h = 128;
    rect2D(x,y,w,h,0.06f,0.04f,0.10f,0.84f);
    border2D(x,y,w,h,0.70f,0.45f,0.95f,0.72f);
    char buf[160];
    text2DShadow(x+12,y+h-18,"Galaxy Diagnostics",0.90f,0.70f,1.0f,GLUT_BITMAP_HELVETICA_18);
    std::snprintf(buf,sizeof(buf),"Stars: %d  Nebulae: %d  SFR: %d  Nearby: %d",NSTARS,NNEB,N_SFR,N_NEARBY_SYSTEMS);
    text2D(x+12,y+h-42,buf,0.88f,0.90f,1.0f,GLUT_BITMAP_HELVETICA_12);
    std::snprintf(buf,sizeof(buf),"Outer worlds: %d  Deep galaxies: %d  Quasars: %d",N_OUTER_BODIES,N_DEEP_GALAXIES,N_QUASARS);
    text2D(x+12,y+h-60,buf,0.88f,0.90f,1.0f,GLUT_BITMAP_HELVETICA_10);
    std::snprintf(buf,sizeof(buf),"Camera Dist: %.1f  Galactic Time: %.3f  Explore Slot: %d/%d",cam.dist,galacticTime,currentExploreIndex+1,totalExploreTargets());
    text2D(x+12,y+h-78,buf,0.88f,0.90f,1.0f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+12,y+h-96,"Control panel is now a live two-column dashboard with status bars.",0.86f,0.90f,0.70f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+12,y+h-112,"Use [ and ] or PageUp/PageDown to jump beyond the Solar System cleanly.",0.78f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}


static void drawConstellationGuide()
{
    if(!showConstellationGuide) return;
    beginOrtho();
    rect2D(390,10,340,68,0.06f,0.08f,0.14f,0.80f);
    border2D(390,10,340,68,0.45f,0.60f,0.95f,0.70f);
    text2DShadow(404,56,"Visualization Strategy",0.96f,0.96f,1.0f,GLUT_BITMAP_HELVETICA_18);
    text2D(404,36,"Depth fade -> focus weight -> zoom-aware HUD",0.72f,0.86f,1.0f,GLUT_BITMAP_HELVETICA_12);
    text2D(404,20,"This reduces clutter instead of stacking more effects.",0.78f,0.88f,0.72f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}

static void drawStellarSurveyPanel()
{
    if(!showStarSurvey) return;
    beginOrtho();
    float x = 390, y = 84, w = 340, h = 196;
    rect2D(x,y,w,h,0.05f,0.06f,0.13f,0.84f);
    border2D(x,y,w,h,0.48f,0.60f,0.95f,0.72f);
    text2DShadow(x+12,y+h-18,"Stellar Survey",0.92f,0.96f,1.0f,GLUT_BITMAP_HELVETICA_18);
    text2D(x+12,y+h-40,"O/B  hot blue stars      | A/F  white to yellow-white",0.70f,0.86f,1.0f,GLUT_BITMAP_HELVETICA_12);
    text2D(x+12,y+h-56,"G/K  Sun-like to orange  | M    red dwarfs dominate",0.98f,0.90f,0.72f,GLUT_BITMAP_HELVETICA_12);
    text2D(x+12,y+h-78,"Nearby systems in this build:",0.82f,0.94f,1.0f,GLUT_BITMAP_HELVETICA_12);
    text2D(x+12,y+h-94,"Proxima, Alpha Cen A/B, Barnard, Sirius, TRAPPIST-1",0.88f,0.92f,0.98f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+12,y+h-110,"Outer frontier objects:",0.82f,0.94f,1.0f,GLUT_BITMAP_HELVETICA_12);
    text2D(x+12,y+h-126,"Haumea, Makemake, Eris, Sedna, and a Planet Nine marker",0.88f,0.92f,0.98f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+12,y+h-148,"Deep space layer keeps the Milky Way from feeling visually isolated.",0.78f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+12,y+h-164,"Use [ ] to jump outward from the Solar System into nearby stars and beyond.",0.90f,0.90f,0.72f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+12,y+16,"Press ' to hide this survey panel.",0.74f,0.82f,0.96f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}


static void drawSystemOverviewPanel()
{
    if(!showSystemOverview) return;
    beginOrtho();
    float x = WIN_W - 410, y = 210, w = 392, h = 236;
    rect2D(x,y,w,h,0.04f,0.07f,0.13f,0.86f);
    border2D(x,y,w,h,0.34f,0.64f,0.96f,0.76f);
    text2DShadow(x+12,y+h-18,"Solar System + Frontier Overview",0.94f,0.96f,1.0f,GLUT_BITMAP_HELVETICA_18);
    float yy = y + h - 40;
    text2D(x+12,yy,"Planet            Dist(M km)  Temp(C)  Moons",0.74f,0.88f,1.0f,GLUT_BITMAP_HELVETICA_12);
    yy -= 16;
    for(int i=0;i<NP;i++) {
        char buf[160];
        std::snprintf(buf,sizeof(buf),"%-16s %8.1f   %7.0f   %4d",planets[i].name,planets[i].realDistance,planets[i].temperature,planets[i].moons);
        float rr = 0.88f, gg = 0.90f, bb = 0.98f;
        if(cam.focusPlanet == i) { rr = 1.00f; gg = 0.96f; bb = 0.62f; }
        else if(infoPlanet == i) { rr = 0.78f; gg = 0.94f; bb = 1.00f; }
        text2D(x+12,yy,buf,rr,gg,bb,GLUT_BITMAP_8_BY_13);
        yy -= 14;
    }
    yy -= 4;
    text2D(x+12,yy,"Beyond Neptune:",0.82f,0.94f,1.0f,GLUT_BITMAP_HELVETICA_12);
    yy -= 14;
    for(int i=0;i<N_OUTER_BODIES;i++) {
        char buf[180];
        std::snprintf(buf,sizeof(buf),"%-18s %s",outerBodies[i].name, outerBodies[i].kind);
        text2D(x+12,yy,buf,0.88f,0.90f,0.98f,GLUT_BITMAP_8_BY_13);
        yy -= 14;
    }
    text2D(x+12,y+18,"Press / to hide this table.",0.74f,0.82f,0.96f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}


static void drawFocusPanel()
{
    if(!showFocusPanel) return;
    beginOrtho();
    float x = 10, y = WIN_H - 312, w = 380, h = 90;
    rect2D(x,y,w,h,0.05f,0.05f,0.11f,0.86f);
    border2D(x,y,w,h,0.60f,0.68f,0.95f,0.74f);
    text2DShadow(x+12,y+h-18,"Focus Target",0.96f,0.96f,1.0f,GLUT_BITMAP_HELVETICA_18);
    if(cam.focusPlanet >= 0) {
        char buf[220];
        std::snprintf(buf,sizeof(buf),"%s | gravity %.2f m/s^2 | temp %.0f C | moons %d",planets[cam.focusPlanet].name,planets[cam.focusPlanet].gravity,planets[cam.focusPlanet].temperature,planets[cam.focusPlanet].moons);
        text2D(x+12,y+h-40,buf,0.90f,0.92f,1.0f,GLUT_BITMAP_HELVETICA_12);
        std::snprintf(buf,sizeof(buf),"Composition: %s",planets[cam.focusPlanet].composition);
        text2D(x+12,y+h-58,buf,0.78f,0.88f,0.78f,GLUT_BITMAP_HELVETICA_10);
        std::snprintf(buf,sizeof(buf),"Orbit radius %.2f | diameter %.0f km",planets[cam.focusPlanet].orbitRadius,planets[cam.focusPlanet].realDiameter);
        text2D(x+12,y+h-74,buf,0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
    } else if(cam.focusPlanet == FOCUS_BLACKHOLE) {
        text2D(x+12,y+h-40,"Sagittarius A* | supermassive black hole at the Milky Way core",0.92f,0.82f,1.0f,GLUT_BITMAP_HELVETICA_12);
        text2D(x+12,y+h-58,"Accretion disk, lensing shell, and bipolar jets are all active.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+12,y+h-74,"This is the galactic anchor when zooming out to the Milky Way scale.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
    } else if(focusIsNearby()) {
        int idx = focusNearbyIndex();
        if(idx >= 0 && idx < N_NEARBY_SYSTEMS) {
            char buf[240];
            std::snprintf(buf,sizeof(buf),"%s | %s | known/plausible planets %d",nearbySystems[idx].name,nearbySystems[idx].classification,nearbySystems[idx].exoplanets);
            text2D(x+12,y+h-40,buf,0.96f,0.90f,0.78f,GLUT_BITMAP_HELVETICA_12);
            text2D(x+12,y+h-58,nearbySystems[idx].note,0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
            std::snprintf(buf,sizeof(buf),"Local-bubble offset: (%.0f, %.0f, %.0f)",nearbySystems[idx].dx,nearbySystems[idx].dy,nearbySystems[idx].dz);
            text2D(x+12,y+h-74,buf,0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
        }
    } else if(focusIsOuterBody()) {
        int idx = focusOuterIndex();
        if(idx >= 0 && idx < N_OUTER_BODIES) {
            char buf[240];
            std::snprintf(buf,sizeof(buf),"%s | %s",outerBodies[idx].name, outerBodies[idx].kind);
            text2D(x+12,y+h-40,buf,0.88f,0.94f,1.0f,GLUT_BITMAP_HELVETICA_12);
            text2D(x+12,y+h-58,outerBodies[idx].note,0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
            std::snprintf(buf,sizeof(buf),"Outer orbit %.1f sim units | inclination %.0f deg",outerBodies[idx].orbitRadius,outerBodies[idx].inclination);
            text2D(x+12,y+h-74,buf,0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
        }
    } else if(focusIsDeepGalaxy()) {
        int idx = focusDeepGalaxyIndex();
        if(idx >= 0 && idx < N_DEEP_GALAXIES) {
            char buf[240];
            std::snprintf(buf,sizeof(buf),"%s | deep-sky analog galaxy",deepGalaxies[idx].name);
            text2D(x+12,y+h-40,buf,0.90f,0.86f,1.0f,GLUT_BITMAP_HELVETICA_12);
            std::snprintf(buf,sizeof(buf),"Scale %.1f | intergalactic position (%.0f, %.0f, %.0f)",deepGalaxies[idx].scale,deepGalaxies[idx].x,deepGalaxies[idx].y,deepGalaxies[idx].z);
            text2D(x+12,y+h-58,buf,0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
            text2D(x+12,y+h-74,"These are artistic analogs used to push the scene beyond a single-galaxy sandbox.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
        }
    } else if(focusIsShip()) {
        text2D(x+12,y+h-40,"Voyager-I | exploration craft in heliocentric cruise",0.40f,0.94f,1.0f,GLUT_BITMAP_HELVETICA_12);
        text2D(x+12,y+h-58,"Solar power, telemetry, engine heat, and radar sweep are all active.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+12,y+h-74,"Use [ ] to revisit the ship after exploring distant targets.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
    } else if(focusIsComet()) {
        text2D(x+12,y+h-40,"Halley's Comet | nucleus, coma, ion tail, and dust tail",0.88f,0.94f,1.0f,GLUT_BITMAP_HELVETICA_12);
        text2D(x+12,y+h-58,"Comet focus makes the small-body frontier feel distinct from the planets.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+12,y+h-74,"Its orbit slices across the planetary architecture on a dramatic inclination.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
    } else {
        text2D(x+12,y+h-40,"Sun / free camera | G-type main-sequence star in Orion Spur",0.98f,0.92f,0.70f,GLUT_BITMAP_HELVETICA_12);
        text2D(x+12,y+h-58,"Use number keys, B, [ ], F, and O to jump between solar and galactic scales.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
        text2D(x+12,y+h-74,"Now the scene includes nearby stars, outer dwarfs, ship, comet, and deep galaxies.",0.78f,0.88f,0.98f,GLUT_BITMAP_HELVETICA_10);
    }
    endOrtho();
}


static void drawShipTelemetryPanel()
{
    if(!showShipTelemetry || !showShip || !ship.active) return;
    beginOrtho();
    float x = WIN_W - 312, y = 84, w = 298, h = 118;
    rect2D(x,y,w,h,0.04f,0.08f,0.14f,0.84f);
    border2D(x,y,w,h,0.30f,0.80f,1.0f,0.76f);
    char buf[160];
    text2DShadow(x+12,y+h-18,"Ship Telemetry",0.40f,0.94f,1.0f,GLUT_BITMAP_HELVETICA_18);
    std::snprintf(buf,sizeof(buf),"Bank %.1f deg | Signal %.0f%% | Heat %.0f%%",ship.bankAngle,ship.commSignal*100.0f,ship.engineHeat*100.0f);
    text2D(x+12,y+h-42,buf,0.88f,0.92f,1.0f,GLUT_BITMAP_HELVETICA_12);
    std::snprintf(buf,sizeof(buf),"Radar %.0f deg | Battery %.0f%% | Solar %.0f%%",std::fmod(ship.radarAngle,360.0f),ship.batteryLevel,ship.solarEfficiency);
    text2D(x+12,y+h-58,buf,0.88f,0.92f,1.0f,GLUT_BITMAP_HELVETICA_12);
    std::snprintf(buf,sizeof(buf),"Position %.2f, %.2f, %.2f",ship.x,ship.y,ship.z);
    text2D(x+12,y+h-74,buf,0.74f,0.84f,0.96f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+12,y+h-90,"Use focus cycling to inspect the ship as one of the beyond-Solar-System waypoints.",0.74f,0.84f,0.96f,GLUT_BITMAP_HELVETICA_10);
    text2D(x+12,y+14,"Press , to hide telemetry.",0.74f,0.82f,0.96f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}


static void drawBeyondPanel()
{
    /* Removed in v10: control panel now carries star, planet, label, and arm focus maps. */
}


static void drawInterplanetaryDustBand()
{
    if(isFarView()) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    for(int band=0; band<4; ++band) {
        float inner = 2.2f + band * 1.6f;
        float outer = inner + 2.8f + band * 0.7f;
        float alpha = 0.030f - band * 0.005f;
        glColor4f(0.95f, 0.82f, 0.46f, alpha);
        glBegin(GL_TRIANGLE_STRIP);
        for(int i=0;i<=180;i++) {
            float a = i * TWO_PI / 180.0f;
            float wobble = 1.0f + 0.02f * std::sin(a*6.0f + sunTime*0.8f + band);
            glVertex3f(sunX + std::cos(a)*outer*wobble, sunY + std::sin(a*2.0f)*0.06f, sunZ + std::sin(a)*outer*wobble);
            glVertex3f(sunX + std::cos(a)*inner*wobble, sunY + std::sin(a*2.0f)*0.03f, sunZ + std::sin(a)*inner*wobble);
        }
        glEnd();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawInterplanetRouteLines()
{
    if(isFarView()) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLineWidth(1.2f);
    glBegin(GL_LINES);
    for(int i=0;i<NP;i++) {
        float focus = focusWeight(planets[i].wx, planets[i].wy, planets[i].wz);
        glColor4f(planets[i].r, planets[i].g, planets[i].b, 0.08f * focus);
        glVertex3f(sunX, sunY, sunZ);
        glVertex3f(planets[i].wx, planets[i].wy, planets[i].wz);
    }
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawPlanetNavigationBeacons()
{
    if(isFarView()) return;
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for(int i=0;i<NP;i++) {
        float fade = depthFade(planets[i].wx, planets[i].wy, planets[i].wz, 8.0f, 260.0f);
        float pulse = 0.72f + 0.28f * std::sin(sunTime*2.2f + i*0.7f);
        float mul = (cam.focusPlanet == i) ? 1.8f : 1.0f;
        glColor4f(planets[i].r, planets[i].g, planets[i].b, 0.10f * pulse * fade * mul);
        glPushMatrix();
        glTranslatef(planets[i].wx, planets[i].wy, planets[i].wz);
        glutSolidSphere(planets[i].size * 2.8f, 12, 12);
        glPopMatrix();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawNearbySystems()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for(int i=0;i<N_NEARBY_SYSTEMS;i++) {
        float fade = depthFade(nearbySystems[i].wx, nearbySystems[i].wy, nearbySystems[i].wz, 20.0f, 420.0f);
        if(fade < 0.04f) continue;
        float tw = 0.75f + 0.25f * std::sin(sunTime*2.0f + i);
        glPushMatrix();
        glTranslatef(nearbySystems[i].wx, nearbySystems[i].wy, nearbySystems[i].wz);
        glColor4f(nearbySystems[i].r, nearbySystems[i].g, nearbySystems[i].b, 0.10f * fade * tw);
        glutSolidSphere(nearbySystems[i].size * 5.2f, 10, 10);
        glColor4f(nearbySystems[i].r, nearbySystems[i].g, nearbySystems[i].b, 0.28f * fade * tw);
        glutSolidSphere(nearbySystems[i].size * 2.2f, 12, 12);
        glColor4f(1.0f, 0.96f, 0.90f, 0.95f * fade);
        glutSolidSphere(nearbySystems[i].size, 12, 12);
        if(focusIsNearby() && focusNearbyIndex() == i) {
            glColor4f(0.90f,0.96f,1.0f,0.55f * fade);
            glLineWidth(1.4f);
            for(int e=0;e<std::max(1, nearbySystems[i].exoplanets); e++) {
                float rr = nearbySystems[i].size * (2.0f + e*0.9f);
                glBegin(GL_LINE_LOOP);
                for(int k=0;k<40;k++) {
                    float a = k*TWO_PI/40.0f;
                    glVertex3f(std::cos(a)*rr, 0.0f, std::sin(a)*rr);
                }
                glEnd();
            }
        }
        glPopMatrix();
        if(focusIsNearby() && focusNearbyIndex() == i) drawFocusPulseAt(nearbySystems[i].wx, nearbySystems[i].wy, nearbySystems[i].wz, nearbySystems[i].size * 3.2f, nearbySystems[i].r, nearbySystems[i].g, nearbySystems[i].b);
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawOuterSystemObjects()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.36f,0.48f,0.70f,0.14f);
    glLineWidth(1.0f);
    for(int ring=0; ring<3; ring++) {
        float rr = 32.0f + ring * 12.0f;
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<180;i++) {
            float a = i*TWO_PI/180.0f;
            glVertex3f(sunX + std::cos(a)*rr, sunY, sunZ + std::sin(a)*rr);
        }
        glEnd();
    }
    glColor4f(0.34f,0.42f,0.58f,0.07f);
    for(int ring=0; ring<2; ring++) {
        float rr = 140.0f + ring * 24.0f;
        glBegin(GL_LINE_LOOP);
        for(int i=0;i<180;i++) {
            float a = i*TWO_PI/180.0f;
            glVertex3f(sunX + std::cos(a)*rr, sunY + (ring?4.0f:-4.0f), sunZ + std::sin(a)*rr);
        }
        glEnd();
    }
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);

    for(int i=0;i<N_OUTER_BODIES;i++) {
        float fade = depthFade(outerBodies[i].wx, outerBodies[i].wy, outerBodies[i].wz, 12.0f, 360.0f);
        if(fade < 0.03f) continue;
        glPushMatrix();
        glTranslatef(outerBodies[i].wx, outerBodies[i].wy, outerBodies[i].wz);
        glColor3f(outerBodies[i].r*fade, outerBodies[i].g*fade, outerBodies[i].b*fade);
        gluSphere(Q, outerBodies[i].size, 18, 18);
        if(focusIsOuterBody() && focusOuterIndex() == i) drawSelectionReticle(outerBodies[i].size * 1.2f);
        glPopMatrix();
        if(focusIsOuterBody() && focusOuterIndex() == i) drawFocusPulseAt(outerBodies[i].wx, outerBodies[i].wy, outerBodies[i].wz, outerBodies[i].size * 6.0f, outerBodies[i].r, outerBodies[i].g, outerBodies[i].b);
    }
}

static void drawZodiacalDustBand()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glPushMatrix();
    glTranslatef(sunX, sunY, sunZ);
    glRotatef(8.0f, 1, 0, 0);
    for(int layer=0; layer<4; ++layer) {
        float inner = 2.4f + layer*2.2f;
        float outer = inner + 5.5f + layer*1.4f;
        float alpha = (0.028f - layer*0.005f) * (isFarView() ? 0.4f : 1.0f);
        glColor4f(0.95f, 0.78f, 0.38f, alpha);
        glBegin(GL_TRIANGLE_STRIP);
        for(int i=0;i<=120;i++) {
            float a = i*TWO_PI/120.0f;
            float wave = 1.0f + 0.03f*std::sin(a*7.0f + layer + sunTime*0.6f);
            glVertex3f(std::cos(a)*outer*wave, 0.0f, std::sin(a)*outer*wave);
            glVertex3f(std::cos(a)*inner*wave, 0.0f, std::sin(a)*inner*wave);
        }
        glEnd();
    }
    glPopMatrix();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawInterworldRoutes()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    for(int i=0;i<NP;i++) {
        float fade = depthFade(planets[i].wx, planets[i].wy, planets[i].wz, 6.0f, 220.0f);
        if(fade < 0.08f) continue;
        glLineWidth((cam.focusPlanet == i) ? 2.4f : 1.0f);
        glBegin(GL_LINE_STRIP);
        for(int s=0;s<=18;s++) {
            float t = (float)s / 18.0f;
            float lift = std::sin(t*PI) * 0.15f * planets[i].orbitRadius;
            float rr = LERP(sunX, planets[i].wx, t);
            float ry = LERP(sunY, planets[i].wy, t) + lift*0.02f;
            float rz = LERP(sunZ, planets[i].wz, t);
            float a = (0.10f + 0.24f*(1.0f-t)) * fade * ((cam.focusPlanet == i) ? 1.2f : 0.6f);
            glColor4f(planets[i].r, planets[i].g, planets[i].b, a);
            glVertex3f(rr, ry, rz);
        }
        glEnd();
    }

    if(cam.focusPlanet >= 0 && cam.focusPlanet < NP) {
        int idx = cam.focusPlanet;
        for(int i=0;i<NP;i++) {
            if(i == idx) continue;
            float d = DIST2(planets[idx].wx, planets[idx].wy, planets[idx].wz, planets[i].wx, planets[i].wy, planets[i].wz);
            if(d > 12.0f) continue;
            glColor4f(0.80f, 0.88f, 1.00f, 0.10f);
            glLineWidth(1.0f);
            glBegin(GL_LINES);
            glVertex3f(planets[idx].wx, planets[idx].wy, planets[idx].wz);
            glVertex3f(planets[i].wx, planets[i].wy, planets[i].wz);
            glEnd();
        }
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawPlanetBeacons()
{
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    for(int i=0;i<NP;i++) {
        float fade = depthFade(planets[i].wx, planets[i].wy, planets[i].wz, 5.0f, 240.0f);
        if(fade < 0.05f) continue;
        float pulse = 0.65f + 0.35f*std::sin(sunTime*2.6f + i);
        glColor4f(planets[i].r, planets[i].g, planets[i].b, 0.12f*fade*pulse);
        glPushMatrix();
        glTranslatef(planets[i].wx, planets[i].wy, planets[i].wz);
        glutSolidSphere(planets[i].size*1.55f + 0.05f, 12, 12);
        glPopMatrix();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

static void drawInterworldVisuals()
{
    if(isFarView()) return;
    drawZodiacalDustBand();
    drawInterworldRoutes();
    drawPlanetBeacons();
}

static void drawDeepSpace()
{
    if(!showDeepSpace) return;
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for(int i=0;i<N_DEEP_GALAXIES;i++) {
        float fade = depthFade(deepGalaxies[i].x,deepGalaxies[i].y,deepGalaxies[i].z,250.0f,2600.0f);
        if(fade < 0.05f) continue;
        glPushMatrix();
        glTranslatef(deepGalaxies[i].x,deepGalaxies[i].y,deepGalaxies[i].z);
        glRotatef(deepGalaxies[i].rotation + sunTime*deepGalaxies[i].spin,0,1,0);
        glRotatef(20.0f + i*7.0f,1,0,0);
        float base = deepGalaxies[i].scale;
        for(int layer=0; layer<5; layer++) {
            float rr = base * (0.32f + 0.20f*layer);
            float aa = (0.08f - 0.012f*layer) * fade;
            glColor4f(deepGalaxies[i].r,deepGalaxies[i].g,deepGalaxies[i].b,aa);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0,0,0);
            for(int k=0;k<=36;k++) {
                float a = k*TWO_PI/36.0f;
                float w = 1.0f + 0.18f*std::sin(a*(deepGalaxies[i].type==0?4.0f:2.0f)+layer);
                glVertex3f(std::cos(a)*rr*w, std::sin(a)*rr*deepGalaxies[i].flatten, 0.0f);
            }
            glEnd();
        }
        glColor4f(1.0f,0.96f,0.88f,0.18f*fade);
        glutSolidSphere(base*0.08f,10,10);
        glPopMatrix();
        if(focusIsDeepGalaxy() && focusDeepGalaxyIndex() == i) drawFocusPulseAt(deepGalaxies[i].x, deepGalaxies[i].y, deepGalaxies[i].z, base * 0.34f, deepGalaxies[i].r, deepGalaxies[i].g, deepGalaxies[i].b);
    }

    glPointSize(3.5f);
    glBegin(GL_POINTS);
    for(int i=0;i<N_QUASARS;i++) {
        float tw = 0.6f + 0.4f*std::sin(sunTime*1.2f + quasars[i].phase);
        glColor4f(quasars[i].r*quasars[i].intensity,quasars[i].g*quasars[i].intensity,quasars[i].b,0.55f*tw);
        glVertex3f(quasars[i].x,quasars[i].y,quasars[i].z);
    }
    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

static void drawLensFlare()
{
    if(!showLensFlare) return;
    glGetDoublev(GL_MODELVIEW_MATRIX,gMV);
    glGetDoublev(GL_PROJECTION_MATRIX,gPR);
    glGetIntegerv(GL_VIEWPORT,gVP);
    GLdouble sx,sy,sz;
    if(gluProject(sunX,sunY,sunZ,gMV,gPR,gVP,&sx,&sy,&sz) != GL_TRUE) return;
    if(sz < 0 || sz > 1) return;
    if(sx < -50 || sx > WIN_W+50 || sy < -50 || sy > WIN_H+50) return;
    float px = (float)sx, py = (float)sy;
    beginOrtho();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float mainPulse = 0.8f + 0.2f * std::sin(sunTime*3.5f);
    for(int r=80;r>=5;r-=8) {
        float alpha = 0.008f * (80-r) * mainPulse;
        glColor4f(1.0f,0.90f,0.60f,alpha);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(px,py);
        for(int k=0;k<=30;k++) {
            float a = k*TWO_PI/30.0f;
            glVertex2f(px+std::cos(a)*r, py+std::sin(a)*r);
        }
        glEnd();
    }
    for(int i=0;i<N_LENS_RAYS;i++) {
        float a = lensRays[i].angle + sunTime*0.04f;
        float len = lensRays[i].length * mainPulse * (0.85f + 0.15f*std::sin(sunTime*2+i));
        glColor4f(lensRays[i].r,lensRays[i].g,lensRays[i].b,lensRays[i].a*mainPulse);
        glLineWidth(lensRays[i].width);
        glBegin(GL_LINES);
        glVertex2f(px,py);
        glVertex2f(px+std::cos(a)*len, py+std::sin(a)*len);
        glEnd();
    }
    glDisable(GL_BLEND);
    endOrtho();
}

static void drawDepthDebug()
{
    if(!showDepthDebug) return;
    beginOrtho();
    rect2D(740,10,260,58,0.15f,0.04f,0.04f,0.78f);
    border2D(740,10,260,58,0.95f,0.35f,0.35f,0.70f);
    char buf[128];
    std::snprintf(buf,sizeof(buf),"Depth Debug: near fade gate %.2f",depthFade(sunX,sunY,sunZ,5.0f,500.0f));
    text2DShadow(752,46,buf,1.0f,0.88f,0.88f,GLUT_BITMAP_HELVETICA_12);
    text2D(752,24,"Use D to toggle this panel.",0.95f,0.82f,0.82f,GLUT_BITMAP_HELVETICA_10);
    endOrtho();
}

static void drawHUD()
{
    if(cam.dist > 120 || cam.focusPlanet >= 0) {
        drawMiniMap();
    }
    drawInfoPanel();
    drawPowerPanel();
    drawShipTelemetryPanel();
    drawScaleAndFPS();
    drawHelpPanel();
    drawGalaxyDiagnosticsPanel();
    drawConstellationGuide();
    drawDepthDebug();
}


static void display()
{
    updateUniverse();
    glViewport(0,0,WIN_W,WIN_H);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)WIN_W / (double)WIN_H, 0.05, 1500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    applyCamera();
    setLightAtSun();

    drawDeepSpace();
    drawGalaxyStars();
    drawNebulae();
    drawSFRs();
    drawBlackHole();
    drawMeteors();
    drawNearbySystems();

    if(showGrid && !isFarView()) {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.18f,0.26f,0.40f,0.18f);
        glLineWidth(0.5f);
        float gs = 45, step = 5;
        glPushMatrix();
        glTranslatef(sunX,sunY,sunZ);
        glBegin(GL_LINES);
        for(float xx=-gs; xx<=gs; xx+=step) {
            glVertex3f(xx,-0.5f,-gs); glVertex3f(xx,-0.5f,gs);
            glVertex3f(-gs,-0.5f,xx); glVertex3f(gs,-0.5f,xx);
        }
        glEnd();
        glPopMatrix();
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    drawInterworldVisuals();

    if(showOrbits && !isFarView()) {
        for(int i=0;i<NP;i++) drawOrbit(planets[i].orbitRadius,0.28f,0.33f,0.46f,0.32f);
        if(cam.focusPlanet >= 0) drawSelectedOrbit(cam.focusPlanet);
    }

    drawPlanetTrails();
    drawSun();
    drawAsteroids();
    drawOuterSystemObjects();
    for(int i=0;i<NP;i++) drawPlanet(i);
    drawComet();
    drawShip();

    glGetDoublev(GL_MODELVIEW_MATRIX,gMV);
    glGetDoublev(GL_PROJECTION_MATRIX,gPR);
    glGetIntegerv(GL_VIEWPORT,gVP);
    drawAllLabels();
    drawArmLabels();
    drawGalaxyTitle();
    drawLensFlare();
    drawHUD();

    glutSwapBuffers();
}


static void reshape(int w,int h)
{
    if(h == 0) h = 1;
    WIN_W = w;
    WIN_H = h;
    glViewport(0,0,w,h);
}

static void keyboard(unsigned char key,int x,int y)
{
    (void)x; (void)y;
    switch(key) {
        case 27: std::exit(0); break;
        case '0': focusSolarTarget(0); break;
        case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            focusSolarTarget((int)(key - '0')); break;
        case 'b': case 'B': focusOnPlanet(FOCUS_BLACKHOLE); break;
        case 'f': case 'F': cam.mode = CAM_FOLLOW; break;
        case 'o': case 'O': if(cam.focusPlanet >= 0) cam.mode = CAM_ORBIT; break;
        case 'p': case 'P': paused = !paused; break;
        case '+': case '=': simSpeed *= 1.22f; if(simSpeed > 150) simSpeed = 150; break;
        case '-': case '_': simSpeed /= 1.22f; if(simSpeed < 0.01f) simSpeed = 0.01f; break;
        case 9:
            if(infoPlanet < 0) infoPlanet = 0; else infoPlanet = (infoPlanet + 1) % NP;
            if(cam.focusPlanet < 0 || cam.focusPlanet >= NP) {
                focusOnPlanet(infoPlanet);
            }
            break;
        case 'm': case 'M': showMap = !showMap; break;
        case 'i': case 'I': showInfo = !showInfo; break;
        case 'h': case 'H': showPowerPanel = !showPowerPanel; break;
        case 't': case 'T': showTrails = !showTrails; break;
        case 'l': case 'L': showLabels = !showLabels; break;
        case 's': case 'S': showScale = !showScale; break;
        case 'c': case 'C': showClouds = !showClouds; break;
        case 'g': case 'G': showGrid = !showGrid; break;
        case 'n': case 'N': showArmNames = !showArmNames; break;
        case 'k': case 'K': showClusters = !showClusters; break;
        case 'v': case 'V': showNebulae = !showNebulae; break;
        case 'z': case 'Z': showSFR = !showSFR; break;
        case 'x': case 'X': showArmLines = !showArmLines; break;
        case 'y': case 'Y': showMeteors = !showMeteors; break;
        case 'u': case 'U': showShip = !showShip; break;
        case 'q': case 'Q': showComet = !showComet; break;
        case 'e': case 'E': showOrbits = !showOrbits; break;
        case 'w': case 'W': nightMode = !nightMode; break;
        case 'j': case 'J': showHelp = !showHelp; break;
        case 'a': case 'A': showGalaxyDiagnostics = !showGalaxyDiagnostics; break;
        case 'd': case 'D': showDepthDebug = !showDepthDebug; break;
        case '`': showConstellationGuide = !showConstellationGuide; break;
        case ';': showDeepSpace = !showDeepSpace; break;
        case ',': showShipTelemetry = !showShipTelemetry; break;
        case '[': case '{': cycleExploreTarget(-1); break;
        case ']': case '}': cycleExploreTarget(1); break;
        case 'r': case 'R':
            cam.dist = 500; cam.yaw = 90; cam.pitch = 75;
            cam.targetX = cam.targetY = cam.targetZ = 0;
            cam.smoothX = cam.smoothY = cam.smoothZ = 0;
            cam.mode = CAM_FREE; cam.focusPlanet = FOCUS_SUN; showInfo = true;
            cam.transitioning = false; simSpeed = 1.0f; infoPlanet = -1;
            showDeepSpace = true;
            showShipTelemetry = true;
            currentExploreIndex = 0;
            break;
        default: break;
    }
}


static void specialKeyboard(int key,int x,int y)
{
    (void)x; (void)y;
    switch(key) {
        case GLUT_KEY_LEFT:  cam.yaw -= 3.0f; break;
        case GLUT_KEY_RIGHT: cam.yaw += 3.0f; break;
        case GLUT_KEY_UP:    cam.pitch += 2.0f; if(cam.pitch>89) cam.pitch=89; break;
        case GLUT_KEY_DOWN:  cam.pitch -= 2.0f; if(cam.pitch<-89) cam.pitch=-89; break;
#ifdef GLUT_KEY_PAGE_UP
        case GLUT_KEY_PAGE_UP: cycleExploreTarget(1); break;
#endif
#ifdef GLUT_KEY_PAGE_DOWN
        case GLUT_KEY_PAGE_DOWN: cycleExploreTarget(-1); break;
#endif
        default: break;
    }
}


static void mouse(int button,int state,int x,int y)
{
    if(button == GLUT_LEFT_BUTTON) {
        mouseDown = (state == GLUT_DOWN);
        lastMouseX = x; lastMouseY = y;
    }
    if(button == GLUT_RIGHT_BUTTON) {
        rightDown = (state == GLUT_DOWN);
        mouseRX = x; mouseRY = y;
    }
    if(state == GLUT_DOWN) {
        if(button == 3) {
            cam.dist -= cam.dist * 0.14f;
            if(cam.dist < 0.8f) cam.dist = 0.8f;
        } else if(button == 4) {
            cam.dist += cam.dist * 0.14f;
            if(cam.dist > 1100) cam.dist = 1100;
        }
    }
}

static void motion(int x,int y)
{
    if(mouseDown) {
        int dx = x - lastMouseX, dy = y - lastMouseY;
        lastMouseX = x; lastMouseY = y;
        cam.yaw += dx * 0.33f;
        cam.pitch -= dy * 0.26f;
        cam.pitch = CLAMP(cam.pitch,-89,89);
        if(cam.mode == CAM_ORBIT && cam.focusPlanet >= 0) cam.mode = CAM_FOLLOW;
    }
    if(rightDown) {
        int dx = x - mouseRX, dy = y - mouseRY;
        mouseRX = x; mouseRY = y;
        float panScale = cam.dist * 0.002f;
        cam.targetX -= dx * panScale * std::cos(DEG2RAD(cam.yaw));
        cam.targetZ -= dx * panScale * std::sin(DEG2RAD(cam.yaw));
        cam.targetY += dy * panScale * 0.5f;
    }
}

static void idle()
{
    ++frameCounter;
    ++fpsFrames;
    int now = glutGet(GLUT_ELAPSED_TIME);
    if(now - lastFPSTime >= 1000) {
        fps = fpsFrames;
        fpsFrames = 0;
        lastFPSTime = now;
    }
    glutPostRedisplay();
}

int main(int argc,char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(40, 20);
    glutCreateWindow("Galactic Voyager v11.0 - Precision Focus and Visual Upgrade");
    initGL();
    lastFPSTime = glutGet(GLUT_ELAPSED_TIME);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    glutMainLoop();
    return 0;
}
