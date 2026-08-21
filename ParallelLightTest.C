#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#include "TRandom.h"
#include "parallel_light.h"
#include "parallel_light_tracer.h"
#include "diffuse_target.h"
#include "detector.h"

// ============================================================
// User adjustable parameters
// ============================================================

// -------------------- Monte Carlo --------------------
static const long long TOTAL_PHOTONS = 1000000000000LL;  // 1e12
static const long long BATCH_SIZE = 100000000LL;         // 1e8
static const int MAX_WORKERS = 16;
static const uint64_t MASTER_SEED = 202608190001ULL;

// -------------------- Parallel light source --------------------
static const double SOURCE_X = 0.0;          // mm
static const double SOURCE_Y = 0.0;          // mm
static const double SOURCE_Z = 5800.0;       // mm
static const double SOURCE_RADIUS = 2000.0;  // mm
static const double WAVELENGTH = 400.0;      // nm

// -------------------- Diffuse target --------------------
static const double TARGET_X = 0.0;             // mm
static const double TARGET_Y = 0.0;             // mm
static const double TARGET_Z = 2870.0;          // mm
static const double TARGET_WIDTH = 200.0;       // mm
static const double TARGET_HEIGHT = 200.0;      // mm
static const double TARGET_REFLECTIVITY = 0.98;

// -------------------- CCD position --------------------
// CCD_X/Y/Z = center of the 170 mm circular body
static const double CCD_X = 0.0;  // mm
static const double CCD_Y = 0.0;  // mm
static const double CCD_Z = 0.0;  // mm

// -------------------- CCD LookAt --------------------
static const double CCD_LOOKAT_X = 0.0;     // mm
static const double CCD_LOOKAT_Y = 0.0;     // mm
static const double CCD_LOOKAT_Z = 2870.0;  // mm

// -------------------- CCD optics --------------------
static const double CCD_FOCAL_LENGTH = 35.0;     // mm
static const double CCD_F_NUMBER = 1.8;
static const double CCD_HORIZONTAL_FOV = 14.33;  // deg
static const double CCD_VERTICAL_FOV = 10.77;    // deg

// -------------------- CCD mechanical geometry --------------------
static const double CCD_DISK_DIAMETER = 170.0;       // mm
static const double CCD_BASE_WIDTH = 140.0;           // mm
static const double CCD_BASE_HEIGHT = 15.0;           // mm
static const double CCD_BASE_LEFT_OFFSET = 6.0;       // mm
static const double CCD_HOLE_DIAMETER = 21.0;         // mm
static const double CCD_HOLE_EDGE_DISTANCE = 13.0;    // mm

// -------------------- Parallel-light obstructions --------------------
static const bool ENABLE_DOOR = true;
static const bool ENABLE_CAMERA = true;
static const bool ENABLE_DUCT = true;
static const bool ENABLE_DUCT_STAIR = true;
static const bool ENABLE_LED_FRAME = true;
static const bool ENABLE_CONTAINER_WALL = true;

// -------------------- Lambert-return obstructions --------------------
static const bool LAMBERT_ENABLE_DOOR = false;
static const bool LAMBERT_ENABLE_CAMERA = false;
static const bool LAMBERT_ENABLE_DUCT = true;
static const bool LAMBERT_ENABLE_DUCT_STAIR = true;
static const bool LAMBERT_ENABLE_LED_FRAME = true;
static const bool LAMBERT_ENABLE_CONTAINER_WALL = true;

// -------------------- Output --------------------
static const char *RESULT_DIR = "mc_results_detector_body";
static const char *SEED_FILE = "mc_results_detector_body/random_seeds.txt";
static const char *SUMMARY_FILE = "mc_results_detector_body/summary.txt";

// ============================================================
// Counters
// ============================================================

struct SimulationCounters
{
    long long photons = 0;

    long long blocked_door = 0;
    long long blocked_camera = 0;
    long long blocked_duct = 0;
    long long blocked_stair = 0;
    long long blocked_led = 0;
    long long blocked_container = 0;
    long long unknown_status = 0;

    long long detector_parallel_tested = 0;
    long long detector_parallel_accepted = 0;
    long long detector_parallel_blocked_body = 0;
    long long detector_parallel_outside_body = 0;
    long long detector_parallel_miss_aperture = 0;
    long long detector_parallel_outside_fov = 0;
    long long detector_parallel_wrong_direction = 0;
    long long detector_parallel_invalid_geometry = 0;
    long long detector_parallel_unknown = 0;

    long long miss_mirror = 0;
    long long hit_mirror = 0;
    long long mirror_reflected = 0;
    long long mirror_absorbed = 0;

    long long hit_target = 0;
    long long miss_target = 0;
    long long target_trace_error = 0;

    long long target_reflected = 0;
    long long target_absorbed = 0;

    long long detector_target_tested = 0;
    long long detector_target_accepted = 0;
    long long detector_target_blocked_body = 0;
    long long detector_target_outside_body = 0;
    long long detector_target_miss_aperture = 0;
    long long detector_target_outside_fov = 0;
    long long detector_target_wrong_direction = 0;
    long long detector_target_invalid_geometry = 0;
    long long detector_target_unknown = 0;

    long long lambert_blocked_duct = 0;
    long long lambert_blocked_stair = 0;
    long long lambert_blocked_led = 0;
    long long lambert_blocked_container = 0;
    long long lambert_miss_mirror = 0;
    long long lambert_hit_mirror = 0;
    long long lambert_invalid_direction = 0;
    long long lambert_unknown_status = 0;

    void Add(const SimulationCounters &x)
    {
        photons += x.photons;

        blocked_door += x.blocked_door;
        blocked_camera += x.blocked_camera;
        blocked_duct += x.blocked_duct;
        blocked_stair += x.blocked_stair;
        blocked_led += x.blocked_led;
        blocked_container += x.blocked_container;
        unknown_status += x.unknown_status;

        detector_parallel_tested += x.detector_parallel_tested;
        detector_parallel_accepted += x.detector_parallel_accepted;
        detector_parallel_blocked_body += x.detector_parallel_blocked_body;
        detector_parallel_outside_body += x.detector_parallel_outside_body;
        detector_parallel_miss_aperture += x.detector_parallel_miss_aperture;
        detector_parallel_outside_fov += x.detector_parallel_outside_fov;
        detector_parallel_wrong_direction += x.detector_parallel_wrong_direction;
        detector_parallel_invalid_geometry += x.detector_parallel_invalid_geometry;
        detector_parallel_unknown += x.detector_parallel_unknown;

        miss_mirror += x.miss_mirror;
        hit_mirror += x.hit_mirror;
        mirror_reflected += x.mirror_reflected;
        mirror_absorbed += x.mirror_absorbed;

        hit_target += x.hit_target;
        miss_target += x.miss_target;
        target_trace_error += x.target_trace_error;

        target_reflected += x.target_reflected;
        target_absorbed += x.target_absorbed;

        detector_target_tested += x.detector_target_tested;
        detector_target_accepted += x.detector_target_accepted;
        detector_target_blocked_body += x.detector_target_blocked_body;
        detector_target_outside_body += x.detector_target_outside_body;
        detector_target_miss_aperture += x.detector_target_miss_aperture;
        detector_target_outside_fov += x.detector_target_outside_fov;
        detector_target_wrong_direction += x.detector_target_wrong_direction;
        detector_target_invalid_geometry += x.detector_target_invalid_geometry;
        detector_target_unknown += x.detector_target_unknown;

        lambert_blocked_duct += x.lambert_blocked_duct;
        lambert_blocked_stair += x.lambert_blocked_stair;
        lambert_blocked_led += x.lambert_blocked_led;
        lambert_blocked_container += x.lambert_blocked_container;
        lambert_miss_mirror += x.lambert_miss_mirror;
        lambert_hit_mirror += x.lambert_hit_mirror;
        lambert_invalid_direction += x.lambert_invalid_direction;
        lambert_unknown_status += x.lambert_unknown_status;
    }
};

// ============================================================
// Random seed
// ============================================================

uint64_t SplitMix64(uint64_t x)
{
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

unsigned int GetBatchSeed(long long batch_id)
{
    uint64_t value = SplitMix64(MASTER_SEED + static_cast<uint64_t>(batch_id));
    unsigned int seed = static_cast<unsigned int>(value & 0xffffffffULL);
    return seed == 0 ? 1U : seed;
}

// ============================================================
// Files
// ============================================================

std::string BatchFileName(long long batch_id)
{
    std::ostringstream ss;
    ss << RESULT_DIR << "/batch_" << std::setw(6)
       << std::setfill('0') << batch_id << ".txt";
    return ss.str();
}

bool FileExists(const std::string &filename)
{
    struct stat buffer;
    return stat(filename.c_str(), &buffer) == 0;
}

bool CreateResultDirectory()
{
    struct stat st;

    if(stat(RESULT_DIR, &st) == 0)
        return S_ISDIR(st.st_mode);

    return mkdir(RESULT_DIR, 0755) == 0;
}

// ============================================================
// Save batch
// ============================================================

bool SaveBatchResult(long long batch_id, unsigned int seed,
                     const SimulationCounters &c)
{
    std::string final_file = BatchFileName(batch_id);

    std::ostringstream ss;
    ss << final_file << ".tmp." << getpid();

    std::string temp_file = ss.str();
    std::ofstream fout(temp_file.c_str());

    if(!fout)
        return false;

    fout << "batch_id " << batch_id << '\n';
    fout << "seed " << seed << '\n';
    fout << "photons " << c.photons << '\n';

    fout << "blocked_door " << c.blocked_door << '\n';
    fout << "blocked_camera " << c.blocked_camera << '\n';
    fout << "blocked_duct " << c.blocked_duct << '\n';
    fout << "blocked_stair " << c.blocked_stair << '\n';
    fout << "blocked_led " << c.blocked_led << '\n';
    fout << "blocked_container " << c.blocked_container << '\n';
    fout << "unknown_status " << c.unknown_status << '\n';

    fout << "detector_parallel_tested " << c.detector_parallel_tested << '\n';
    fout << "detector_parallel_accepted " << c.detector_parallel_accepted << '\n';
    fout << "detector_parallel_blocked_body " << c.detector_parallel_blocked_body << '\n';
    fout << "detector_parallel_outside_body " << c.detector_parallel_outside_body << '\n';
    fout << "detector_parallel_miss_aperture " << c.detector_parallel_miss_aperture << '\n';
    fout << "detector_parallel_outside_fov " << c.detector_parallel_outside_fov << '\n';
    fout << "detector_parallel_wrong_direction " << c.detector_parallel_wrong_direction << '\n';
    fout << "detector_parallel_invalid_geometry " << c.detector_parallel_invalid_geometry << '\n';
    fout << "detector_parallel_unknown " << c.detector_parallel_unknown << '\n';

    fout << "miss_mirror " << c.miss_mirror << '\n';
    fout << "hit_mirror " << c.hit_mirror << '\n';
    fout << "mirror_reflected " << c.mirror_reflected << '\n';
    fout << "mirror_absorbed " << c.mirror_absorbed << '\n';

    fout << "hit_target " << c.hit_target << '\n';
    fout << "miss_target " << c.miss_target << '\n';
    fout << "target_trace_error " << c.target_trace_error << '\n';

    fout << "target_reflected " << c.target_reflected << '\n';
    fout << "target_absorbed " << c.target_absorbed << '\n';

    fout << "detector_target_tested " << c.detector_target_tested << '\n';
    fout << "detector_target_accepted " << c.detector_target_accepted << '\n';
    fout << "detector_target_blocked_body " << c.detector_target_blocked_body << '\n';
    fout << "detector_target_outside_body " << c.detector_target_outside_body << '\n';
    fout << "detector_target_miss_aperture " << c.detector_target_miss_aperture << '\n';
    fout << "detector_target_outside_fov " << c.detector_target_outside_fov << '\n';
    fout << "detector_target_wrong_direction " << c.detector_target_wrong_direction << '\n';
    fout << "detector_target_invalid_geometry " << c.detector_target_invalid_geometry << '\n';
    fout << "detector_target_unknown " << c.detector_target_unknown << '\n';

    fout << "lambert_blocked_duct " << c.lambert_blocked_duct << '\n';
    fout << "lambert_blocked_stair " << c.lambert_blocked_stair << '\n';
    fout << "lambert_blocked_led " << c.lambert_blocked_led << '\n';
    fout << "lambert_blocked_container " << c.lambert_blocked_container << '\n';
    fout << "lambert_miss_mirror " << c.lambert_miss_mirror << '\n';
    fout << "lambert_hit_mirror " << c.lambert_hit_mirror << '\n';
    fout << "lambert_invalid_direction " << c.lambert_invalid_direction << '\n';
    fout << "lambert_unknown_status " << c.lambert_unknown_status << '\n';

    fout.close();

    if(!fout)
    {
        std::remove(temp_file.c_str());
        return false;
    }

    if(std::rename(temp_file.c_str(), final_file.c_str()) != 0)
    {
        std::remove(temp_file.c_str());
        return false;
    }

    return true;
}

// ============================================================
// Load batch
// ============================================================

bool LoadBatchResult(long long batch_id, SimulationCounters &c)
{
    std::ifstream fin(BatchFileName(batch_id).c_str());

    if(!fin)
        return false;

    std::string key;
    long long value;

    while(fin >> key >> value)
    {
        if(key == "photons") c.photons = value;

        else if(key == "blocked_door") c.blocked_door = value;
        else if(key == "blocked_camera") c.blocked_camera = value;
        else if(key == "blocked_duct") c.blocked_duct = value;
        else if(key == "blocked_stair") c.blocked_stair = value;
        else if(key == "blocked_led") c.blocked_led = value;
        else if(key == "blocked_container") c.blocked_container = value;
        else if(key == "unknown_status") c.unknown_status = value;

        else if(key == "detector_parallel_tested") c.detector_parallel_tested = value;
        else if(key == "detector_parallel_accepted") c.detector_parallel_accepted = value;
        else if(key == "detector_parallel_blocked_body") c.detector_parallel_blocked_body = value;
        else if(key == "detector_parallel_outside_body") c.detector_parallel_outside_body = value;
        else if(key == "detector_parallel_miss_aperture") c.detector_parallel_miss_aperture = value;
        else if(key == "detector_parallel_outside_fov") c.detector_parallel_outside_fov = value;
        else if(key == "detector_parallel_wrong_direction") c.detector_parallel_wrong_direction = value;
        else if(key == "detector_parallel_invalid_geometry") c.detector_parallel_invalid_geometry = value;
        else if(key == "detector_parallel_unknown") c.detector_parallel_unknown = value;

        else if(key == "miss_mirror") c.miss_mirror = value;
        else if(key == "hit_mirror") c.hit_mirror = value;
        else if(key == "mirror_reflected") c.mirror_reflected = value;
        else if(key == "mirror_absorbed") c.mirror_absorbed = value;

        else if(key == "hit_target") c.hit_target = value;
        else if(key == "miss_target") c.miss_target = value;
        else if(key == "target_trace_error") c.target_trace_error = value;

        else if(key == "target_reflected") c.target_reflected = value;
        else if(key == "target_absorbed") c.target_absorbed = value;

        else if(key == "detector_target_tested") c.detector_target_tested = value;
        else if(key == "detector_target_accepted") c.detector_target_accepted = value;
        else if(key == "detector_target_blocked_body") c.detector_target_blocked_body = value;
        else if(key == "detector_target_outside_body") c.detector_target_outside_body = value;
        else if(key == "detector_target_miss_aperture") c.detector_target_miss_aperture = value;
        else if(key == "detector_target_outside_fov") c.detector_target_outside_fov = value;
        else if(key == "detector_target_wrong_direction") c.detector_target_wrong_direction = value;
        else if(key == "detector_target_invalid_geometry") c.detector_target_invalid_geometry = value;
        else if(key == "detector_target_unknown") c.detector_target_unknown = value;

        else if(key == "lambert_blocked_duct") c.lambert_blocked_duct = value;
        else if(key == "lambert_blocked_stair") c.lambert_blocked_stair = value;
        else if(key == "lambert_blocked_led") c.lambert_blocked_led = value;
        else if(key == "lambert_blocked_container") c.lambert_blocked_container = value;
        else if(key == "lambert_miss_mirror") c.lambert_miss_mirror = value;
        else if(key == "lambert_hit_mirror") c.lambert_hit_mirror = value;
        else if(key == "lambert_invalid_direction") c.lambert_invalid_direction = value;
        else if(key == "lambert_unknown_status") c.lambert_unknown_status = value;
    }

    return true;
}

// ============================================================
// One batch
// ============================================================

SimulationCounters RunBatch(
    long long photons,
    unsigned int seed,
    WTelescope &telescope,
    ParallelLight &source,
    DiffuseTarget &target,
    Detector &detector,
    ParallelLightTracer &tracer,
    const ObstructionSwitch &sw,
    const ObstructionSwitch &lambert_sw)
{
    SimulationCounters c;
    c.photons = photons;

    gRandom->SetSeed(seed);

    for(long long i = 0; i < photons; ++i)
    {
        double x0, y0, z0;
        double m, n, l;

        source.GenerateRay(x0, y0, z0, m, n, l);

        double xmirror = 0.0;
        double ymirror = 0.0;
        double zmirror = 0.0;
        int mirror_i = -1;
        int mirror_m = -1;

        int status = tracer.TraceToMirror(
            x0, y0, z0,
            m, n, l,
            xmirror, ymirror, zmirror,
            mirror_i, mirror_m,
            sw
        );

        // ----------------------------------------------------
        // Telescope mechanical obstructions
        // ----------------------------------------------------

        if(status != TRACE_HIT_MIRROR &&
           status != TRACE_MISS_MIRROR)
        {
            switch(status)
            {
                case TRACE_BLOCKED_DOOR: ++c.blocked_door; break;
                case TRACE_BLOCKED_CAMERA: ++c.blocked_camera; break;
                case TRACE_BLOCKED_DUCT: ++c.blocked_duct; break;
                case TRACE_BLOCKED_DUCT_STAIR: ++c.blocked_stair; break;
                case TRACE_BLOCKED_LED: ++c.blocked_led; break;
                case TRACE_BLOCKED_CONTAINER: ++c.blocked_container; break;
                default: ++c.unknown_status; break;
            }

            continue;
        }

        // ----------------------------------------------------
        // Parallel light -> CCD
        //
        // CCD is also a physical obstruction for the mirror.
        // ----------------------------------------------------

        ++c.detector_parallel_tested;

        double detector_hit_x = 0.0;
        double detector_hit_y = 0.0;
        double detector_hit_z = 0.0;

        int detector_status = detector.CheckRay(
            x0, y0, z0,
            m, n, l,
            detector_hit_x,
            detector_hit_y,
            detector_hit_z
        );

        switch(detector_status)
        {
            case DETECTOR_ACCEPTED:
                ++c.detector_parallel_accepted;
                break;

            case DETECTOR_BLOCKED_BODY:
                ++c.detector_parallel_blocked_body;
                break;

            case DETECTOR_OUTSIDE_BODY:
                ++c.detector_parallel_outside_body;
                break;

            case DETECTOR_MISS_APERTURE:
                ++c.detector_parallel_miss_aperture;
                break;

            case DETECTOR_OUTSIDE_FOV:
                ++c.detector_parallel_outside_fov;
                break;

            case DETECTOR_WRONG_DIRECTION:
                ++c.detector_parallel_wrong_direction;
                break;

            case DETECTOR_INVALID_GEOMETRY:
                ++c.detector_parallel_invalid_geometry;
                break;

            default:
                ++c.detector_parallel_unknown;
                break;
        }

        // Photon terminates if it interacts with the CCD.
        if(detector_status == DETECTOR_ACCEPTED ||
           detector_status == DETECTOR_BLOCKED_BODY ||
           detector_status == DETECTOR_MISS_APERTURE ||
           detector_status == DETECTOR_OUTSIDE_FOV)
        {
            continue;
        }

        // ----------------------------------------------------
        // Parallel light -> first mirror
        // ----------------------------------------------------

        if(status == TRACE_MISS_MIRROR)
        {
            ++c.miss_mirror;
            continue;
        }

        if(status != TRACE_HIT_MIRROR)
        {
            ++c.unknown_status;
            continue;
        }

        ++c.hit_mirror;

        if(!telescope.Reflected(source.GetWavelength()))
        {
            ++c.mirror_absorbed;
            continue;
        }

        ++c.mirror_reflected;

        // ----------------------------------------------------
        // First mirror -> diffuse target
        // ----------------------------------------------------

        double xtarget = 0.0;
        double ytarget = 0.0;
        double m2 = 0.0;
        double n2 = 0.0;
        double l2 = 0.0;

        int target_status = tracer.TraceMirrorToDiffuseTarget(
            xmirror, ymirror, zmirror,
            mirror_i, mirror_m,
            m, n, l,
            xtarget, ytarget,
            m2, n2, l2
        );

        if(target_status == -2)
        {
            ++c.miss_target;
            continue;
        }

        if(target_status != TRACE_HIT_MIRROR)
        {
            ++c.target_trace_error;
            continue;
        }

        ++c.hit_target;

        // ----------------------------------------------------
        // Diffuse target reflection
        // ----------------------------------------------------

        double md = 0.0;
        double nd = 0.0;
        double ld = 0.0;

        if(!target.Reflect(md, nd, ld))
        {
            ++c.target_absorbed;
            continue;
        }

        ++c.target_reflected;

        // ----------------------------------------------------
        // Lambert return path
        // ----------------------------------------------------

        double xmirror_lambert = 0.0;
        double ymirror_lambert = 0.0;
        double zmirror_lambert = 0.0;
        int lambert_mirror_i = -1;
        int lambert_mirror_m = -1;

        int lambert_status = tracer.TraceLambertToMirror(
            xtarget,
            ytarget,
            target.GetCenterZ(),
            md, nd, ld,
            xmirror_lambert,
            ymirror_lambert,
            zmirror_lambert,
            lambert_mirror_i,
            lambert_mirror_m,
            lambert_sw
        );

        if(lambert_status != TRACE_HIT_MIRROR &&
           lambert_status != TRACE_MISS_MIRROR)
        {
            switch(lambert_status)
            {
                case TRACE_BLOCKED_DUCT:
                    ++c.lambert_blocked_duct;
                    break;

                case TRACE_BLOCKED_DUCT_STAIR:
                    ++c.lambert_blocked_stair;
                    break;

                case TRACE_BLOCKED_LED:
                    ++c.lambert_blocked_led;
                    break;

                case TRACE_BLOCKED_CONTAINER:
                    ++c.lambert_blocked_container;
                    break;

                case TRACE_INVALID_DIRECTION:
                    ++c.lambert_invalid_direction;
                    break;

                default:
                    ++c.lambert_unknown_status;
                    break;
            }

            continue;
        }

        // ----------------------------------------------------
        // Diffuse target -> CCD
        //
        // CCD also blocks the second-mirror path.
        // ----------------------------------------------------

        ++c.detector_target_tested;

        double target_detector_hit_x = 0.0;
        double target_detector_hit_y = 0.0;
        double target_detector_hit_z = 0.0;

        int detector_target_status = detector.CheckRay(
            xtarget,
            ytarget,
            target.GetCenterZ(),
            md, nd, ld,
            target_detector_hit_x,
            target_detector_hit_y,
            target_detector_hit_z
        );

        switch(detector_target_status)
        {
            case DETECTOR_ACCEPTED:
                ++c.detector_target_accepted;
                break;

            case DETECTOR_BLOCKED_BODY:
                ++c.detector_target_blocked_body;
                break;

            case DETECTOR_OUTSIDE_BODY:
                ++c.detector_target_outside_body;
                break;

            case DETECTOR_MISS_APERTURE:
                ++c.detector_target_miss_aperture;
                break;

            case DETECTOR_OUTSIDE_FOV:
                ++c.detector_target_outside_fov;
                break;

            case DETECTOR_WRONG_DIRECTION:
                ++c.detector_target_wrong_direction;
                break;

            case DETECTOR_INVALID_GEOMETRY:
                ++c.detector_target_invalid_geometry;
                break;

            default:
                ++c.detector_target_unknown;
                break;
        }

        if(detector_target_status == DETECTOR_ACCEPTED ||
           detector_target_status == DETECTOR_BLOCKED_BODY ||
           detector_target_status == DETECTOR_MISS_APERTURE ||
           detector_target_status == DETECTOR_OUTSIDE_FOV)
        {
            continue;
        }

        // ----------------------------------------------------
        // Diffuse target -> second mirror
        // ----------------------------------------------------

        if(lambert_status == TRACE_HIT_MIRROR)
            ++c.lambert_hit_mirror;

        else if(lambert_status == TRACE_MISS_MIRROR)
            ++c.lambert_miss_mirror;

        else
            ++c.lambert_unknown_status;
    }

    return c;
}

// ============================================================
// Seed table
// ============================================================

void SaveSeedTable(long long num_batches)
{
    std::ofstream fout(SEED_FILE);

    fout << "# WFCTA Monte Carlo random seeds\n";
    fout << "# MASTER_SEED " << MASTER_SEED << '\n';
    fout << "# TOTAL_PHOTONS " << TOTAL_PHOTONS << '\n';
    fout << "# BATCH_SIZE " << BATCH_SIZE << '\n';
    fout << "# NUM_BATCHES " << num_batches << '\n';
    fout << "# batch_id seed\n";

    for(long long batch = 0; batch < num_batches; ++batch)
        fout << batch << ' ' << GetBatchSeed(batch) << '\n';
}

// ============================================================
// Summary
// ============================================================

void SaveSummary(const SimulationCounters &c,
                 long long completed_batches,
                 long long total_batches)
{
    std::ofstream fout(SUMMARY_FILE);
    fout << std::setprecision(12);

    fout << "============================================================\n";
    fout << "WFCTA Parallel Monte Carlo\n";
    fout << "============================================================\n";
    fout << "Total photons                  : " << TOTAL_PHOTONS << '\n';
    fout << "Completed photons              : " << c.photons << '\n';
    fout << "Completed batches              : " << completed_batches
         << " / " << total_batches << '\n';
    fout << "Master seed                    : " << MASTER_SEED << '\n';

    fout << "------------------------------------------------------------\n";
    fout << "[CCD geometry]\n";
    fout << "CCD center                     : (" << CCD_X << ", " << CCD_Y << ", " << CCD_Z << ") mm\n";
    fout << "CCD LookAt                     : (" << CCD_LOOKAT_X << ", " << CCD_LOOKAT_Y << ", " << CCD_LOOKAT_Z << ") mm\n";
    fout << "Disk diameter                  : " << CCD_DISK_DIAMETER << " mm\n";
    fout << "Base                           : " << CCD_BASE_WIDTH << " x " << CCD_BASE_HEIGHT << " mm\n";
    fout << "Base left offset               : " << CCD_BASE_LEFT_OFFSET << " mm\n";
    fout << "Physical hole diameter         : " << CCD_HOLE_DIAMETER << " mm\n";
    fout << "Hole edge distance             : " << CCD_HOLE_EDGE_DISTANCE << " mm\n";
    fout << "Effective aperture diameter    : " << CCD_FOCAL_LENGTH / CCD_F_NUMBER << " mm\n";

    fout << "------------------------------------------------------------\n";
    fout << "[1] Parallel light -> telescope obstructions\n";
    fout << "Blocked by Door                : " << c.blocked_door << '\n';
    fout << "Blocked by Camera              : " << c.blocked_camera << '\n';
    fout << "Blocked by Duct                : " << c.blocked_duct << '\n';
    fout << "Blocked by Stair               : " << c.blocked_stair << '\n';
    fout << "Blocked by LED                 : " << c.blocked_led << '\n';
    fout << "Blocked Container              : " << c.blocked_container << '\n';

    fout << "------------------------------------------------------------\n";
    fout << "[2] Parallel light -> CCD\n";
    fout << "CCD Tested                     : " << c.detector_parallel_tested << '\n';
    fout << "CCD Accepted                   : " << c.detector_parallel_accepted << '\n';
    fout << "CCD Blocked Body               : " << c.detector_parallel_blocked_body << '\n';
    fout << "CCD Outside Body               : " << c.detector_parallel_outside_body << '\n';
    fout << "CCD Miss Aperture              : " << c.detector_parallel_miss_aperture << '\n';
    fout << "CCD Outside FOV                : " << c.detector_parallel_outside_fov << '\n';

    fout << "------------------------------------------------------------\n";
    fout << "[3] Parallel light -> first mirror\n";
    fout << "Miss Mirror                    : " << c.miss_mirror << '\n';
    fout << "Hit Mirror                     : " << c.hit_mirror << '\n';
    fout << "Mirror Reflected               : " << c.mirror_reflected << '\n';
    fout << "Mirror Absorbed                : " << c.mirror_absorbed << '\n';

    fout << "------------------------------------------------------------\n";
    fout << "[4] First mirror -> diffuse target\n";
    fout << "Hit Target                     : " << c.hit_target << '\n';
    fout << "Miss Target                    : " << c.miss_target << '\n';
    fout << "Target Trace Error             : " << c.target_trace_error << '\n';

    fout << "------------------------------------------------------------\n";
    fout << "[5] Diffuse target reflection\n";
    fout << "Target Reflected               : " << c.target_reflected << '\n';
    fout << "Target Absorbed                : " << c.target_absorbed << '\n';

    fout << "------------------------------------------------------------\n";
    fout << "[6] Diffuse target -> CCD\n";
    fout << "CCD Tested                     : " << c.detector_target_tested << '\n';
    fout << "CCD Accepted                   : " << c.detector_target_accepted << '\n';
    fout << "CCD Blocked Body               : " << c.detector_target_blocked_body << '\n';
    fout << "CCD Outside Body               : " << c.detector_target_outside_body << '\n';
    fout << "CCD Miss Aperture              : " << c.detector_target_miss_aperture << '\n';
    fout << "CCD Outside FOV                : " << c.detector_target_outside_fov << '\n';

    fout << "------------------------------------------------------------\n";
    fout << "[7] Diffuse target -> second mirror\n";
    fout << "Blocked by Duct                : " << c.lambert_blocked_duct << '\n';
    fout << "Blocked by Stair               : " << c.lambert_blocked_stair << '\n';
    fout << "Blocked by LED                 : " << c.lambert_blocked_led << '\n';
    fout << "Blocked Container              : " << c.lambert_blocked_container << '\n';
    fout << "Miss Mirror                    : " << c.lambert_miss_mirror << '\n';
    fout << "Hit Mirror                     : " << c.lambert_hit_mirror << '\n';

    fout << "------------------------------------------------------------\n";
    fout << "[8] Efficiencies\n";

    if(c.photons > 0)
    {
        fout << "Parallel -> CCD / Total        : "
             << static_cast<double>(c.detector_parallel_accepted) / c.photons << '\n';

        fout << "CCD body blocking / Total      : "
             << static_cast<double>(c.detector_parallel_blocked_body) / c.photons << '\n';

        fout << "Parallel -> Mirror / Total     : "
             << static_cast<double>(c.hit_mirror) / c.photons << '\n';

        fout << "Target -> CCD / Total          : "
             << static_cast<double>(c.detector_target_accepted) / c.photons << '\n';

        fout << "Target -> Mirror / Total       : "
             << static_cast<double>(c.lambert_hit_mirror) / c.photons << '\n';
    }

    if(c.hit_mirror > 0)
        fout << "Mirror reflectivity            : "
             << static_cast<double>(c.mirror_reflected) / c.hit_mirror << '\n';

    if(c.mirror_reflected > 0)
        fout << "Mirror -> Target               : "
             << static_cast<double>(c.hit_target) / c.mirror_reflected << '\n';

    if(c.hit_target > 0)
        fout << "Target reflectivity            : "
             << static_cast<double>(c.target_reflected) / c.hit_target << '\n';

    if(c.target_reflected > 0)
    {
        fout << "Target -> CCD                  : "
             << static_cast<double>(c.detector_target_accepted) / c.target_reflected << '\n';

        fout << "Target -> Second Mirror        : "
             << static_cast<double>(c.lambert_hit_mirror) / c.target_reflected << '\n';
    }

    fout << "============================================================\n";
}

// ============================================================
// Main
// ============================================================

int main()
{
    if(!CreateResultDirectory())
    {
        std::cerr << "Error: cannot create result directory.\n";
        return 1;
    }

    const long long NUM_BATCHES =
        (TOTAL_PHOTONS + BATCH_SIZE - 1) / BATCH_SIZE;

    WTelescope telescope;
    telescope.SetMirrorGeometry(1);
    telescope.SetMirror();
    telescope.SetReflectivity(0);

    ParallelLight source(
        SOURCE_X,
        SOURCE_Y,
        SOURCE_Z,
        SOURCE_RADIUS,
        WAVELENGTH
    );

    DiffuseTarget target(
        TARGET_X,
        TARGET_Y,
        TARGET_Z,
        TARGET_WIDTH,
        TARGET_HEIGHT,
        TARGET_REFLECTIVITY
    );

    Detector detector(
        CCD_X,
        CCD_Y,
        CCD_Z,
        CCD_FOCAL_LENGTH,
        CCD_F_NUMBER,
        CCD_HORIZONTAL_FOV,
        CCD_VERTICAL_FOV,
        CCD_DISK_DIAMETER,
        CCD_BASE_WIDTH,
        CCD_BASE_HEIGHT,
        CCD_BASE_LEFT_OFFSET,
        CCD_HOLE_DIAMETER,
        CCD_HOLE_EDGE_DISTANCE
    );

    if(!detector.LookAt(
        CCD_LOOKAT_X,
        CCD_LOOKAT_Y,
        CCD_LOOKAT_Z))
    {
        std::cerr << "Error: invalid detector geometry.\n";
        return 1;
    }

    ParallelLightTracer tracer(&telescope);

    ObstructionSwitch sw;
    sw.door = ENABLE_DOOR;
    sw.camera = ENABLE_CAMERA;
    sw.duct = ENABLE_DUCT;
    sw.duct_stair = ENABLE_DUCT_STAIR;
    sw.led_frame = ENABLE_LED_FRAME;
    sw.container_wall = ENABLE_CONTAINER_WALL;

    ObstructionSwitch lambert_sw;
    lambert_sw.door = LAMBERT_ENABLE_DOOR;
    lambert_sw.camera = LAMBERT_ENABLE_CAMERA;
    lambert_sw.duct = LAMBERT_ENABLE_DUCT;
    lambert_sw.duct_stair = LAMBERT_ENABLE_DUCT_STAIR;
    lambert_sw.led_frame = LAMBERT_ENABLE_LED_FRAME;
    lambert_sw.container_wall = LAMBERT_ENABLE_CONTAINER_WALL;

    SaveSeedTable(NUM_BATCHES);

    std::cout << "============================================================\n";
    std::cout << "WFCTA Parallel Monte Carlo\n";
    std::cout << "============================================================\n";
    std::cout << "Total photons        : " << TOTAL_PHOTONS << '\n';
    std::cout << "Batch size           : " << BATCH_SIZE << '\n';
    std::cout << "Total batches        : " << NUM_BATCHES << '\n';
    std::cout << "Maximum workers      : " << MAX_WORKERS << '\n';
    std::cout << "CCD center           : (" << CCD_X << ", " << CCD_Y << ", " << CCD_Z << ") mm\n";
    std::cout << "CCD LookAt           : (" << CCD_LOOKAT_X << ", " << CCD_LOOKAT_Y << ", " << CCD_LOOKAT_Z << ") mm\n";
    std::cout << "CCD disk             : " << CCD_DISK_DIAMETER << " mm\n";
    std::cout << "CCD base             : " << CCD_BASE_WIDTH << " x " << CCD_BASE_HEIGHT << " mm\n";
    std::cout << "CCD physical hole    : " << CCD_HOLE_DIAMETER << " mm\n";
    std::cout << "CCD optical aperture : " << CCD_FOCAL_LENGTH / CCD_F_NUMBER << " mm\n";
    std::cout << "Camera obstruction   : " << (ENABLE_CAMERA ? "ON" : "OFF") << '\n';
    std::cout << "Master random seed   : " << MASTER_SEED << '\n';
    std::cout << "Result directory     : " << RESULT_DIR << '\n';
    std::cout << "============================================================\n";

    long long next_batch = 0;
    long long completed_batches = 0;

    std::vector<pid_t> running_pids;

    while(next_batch < NUM_BATCHES || !running_pids.empty())
    {
        while(next_batch < NUM_BATCHES &&
              static_cast<int>(running_pids.size()) < MAX_WORKERS)
        {
            if(FileExists(BatchFileName(next_batch)))
            {
                ++completed_batches;
                ++next_batch;
                continue;
            }

            long long batch_id = next_batch;
            long long photons_before = batch_id * BATCH_SIZE;
            long long photons_this_batch = BATCH_SIZE;

            if(photons_before + photons_this_batch > TOTAL_PHOTONS)
                photons_this_batch = TOTAL_PHOTONS - photons_before;

            unsigned int seed = GetBatchSeed(batch_id);

            pid_t pid = fork();

            if(pid < 0)
            {
                std::cerr << "Error: fork failed.\n";
                return 1;
            }

            if(pid == 0)
            {
                SimulationCounters result = RunBatch(
                    photons_this_batch,
                    seed,
                    telescope,
                    source,
                    target,
                    detector,
                    tracer,
                    sw,
                    lambert_sw
                );

                _exit(SaveBatchResult(batch_id, seed, result) ? 0 : 2);
            }

            running_pids.push_back(pid);

            std::cout << "Started batch "
                      << batch_id
                      << "  PID="
                      << pid
                      << "  seed="
                      << seed
                      << '\n';

            ++next_batch;
        }

        if(running_pids.empty())
            continue;

        int child_status = 0;

        pid_t finished =
            wait(&child_status);

        if(finished <= 0)
            continue;

        for(std::vector<pid_t>::iterator it = running_pids.begin();
            it != running_pids.end(); ++it)
        {
            if(*it == finished)
            {
                running_pids.erase(it);
                break;
            }
        }

        if(WIFEXITED(child_status) &&
           WEXITSTATUS(child_status) == 0)
        {
            ++completed_batches;

            double progress =
                100.0 *
                static_cast<double>(completed_batches) /
                static_cast<double>(NUM_BATCHES);

            std::cout << "Completed batches : "
                      << completed_batches
                      << " / "
                      << NUM_BATCHES
                      << "  ("
                      << std::fixed
                      << std::setprecision(3)
                      << progress
                      << " %)"
                      << std::endl;
        }
        else
        {
            std::cerr << "Warning: child process "
                      << finished
                      << " failed.\n";
        }
    }

    SimulationCounters total;
    long long loaded_batches = 0;

    for(long long batch = 0; batch < NUM_BATCHES; ++batch)
    {
        SimulationCounters temp;

        if(LoadBatchResult(batch, temp))
        {
            total.Add(temp);
            ++loaded_batches;
        }
    }

    SaveSummary(
        total,
        loaded_batches,
        NUM_BATCHES
    );

    std::cout << "============================================================\n";
    std::cout << "Simulation finished\n";
    std::cout << "Completed photons    : " << total.photons << '\n';
    std::cout << "Completed batches    : " << loaded_batches << " / " << NUM_BATCHES << '\n';
    std::cout << "Parallel -> CCD      : " << total.detector_parallel_accepted << '\n';
    std::cout << "CCD blocked parallel : " << total.detector_parallel_blocked_body << '\n';
    std::cout << "Target -> CCD        : " << total.detector_target_accepted << '\n';
    std::cout << "Summary              : " << SUMMARY_FILE << '\n';
    std::cout << "============================================================\n";

    return 0;
}
