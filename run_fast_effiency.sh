#!/bin/bash
set -euo pipefail

# ============================================================
# User configuration
# ============================================================

# -------------------- Monte Carlo --------------------
TOTAL_PHOTONS="1000000000000LL"     # 1e12
BATCH_SIZE="100000000LL"            # 1e8
MAX_WORKERS="16"
MASTER_SEED="202608190001ULL"

# -------------------- Parallel light source --------------------
SOURCE_X="0.0"                      # mm
SOURCE_Y="0.0"                      # mm
SOURCE_Z="5800.0"                   # mm
SOURCE_RADIUS="2000.0"              # mm
WAVELENGTH="400.0"                  # nm

# -------------------- Diffuse target --------------------
TARGET_X="0.0"                      # mm
TARGET_Y="0.0"                      # mm
TARGET_Z="2870.0"                   # mm
TARGET_WIDTH="200.0"                # mm
TARGET_HEIGHT="200.0"               # mm
TARGET_REFLECTIVITY="0.98"

# -------------------- CCD position --------------------
# Position of the center of the 170 mm circular body
CCD_X="0.0"                         # mm
CCD_Y="0.0"                         # mm
CCD_Z="0.0"                         # mm

# -------------------- CCD LookAt --------------------
CCD_LOOKAT_X="0.0"                  # mm
CCD_LOOKAT_Y="0.0"                  # mm
CCD_LOOKAT_Z="2870.0"               # mm

# -------------------- CCD optics --------------------
CCD_FOCAL_LENGTH="35.0"             # mm
CCD_F_NUMBER="1.8"
CCD_HORIZONTAL_FOV="14.33"          # deg
CCD_VERTICAL_FOV="10.77"            # deg

# -------------------- CCD mechanical geometry --------------------
CCD_DISK_DIAMETER="170.0"           # mm
CCD_BASE_WIDTH="140.0"              # mm
CCD_BASE_HEIGHT="15.0"              # mm
CCD_BASE_LEFT_OFFSET="6.0"          # mm
CCD_HOLE_DIAMETER="21.0"            # mm
CCD_HOLE_EDGE_DISTANCE="13.0"       # mm

# Effective optical aperture is calculated internally:
# 35 / 1.8 = 19.444... mm

# -------------------- Parallel-light obstructions --------------------
ENABLE_DOOR="true"
ENABLE_CAMERA="true"
ENABLE_DUCT="true"
ENABLE_DUCT_STAIR="true"
ENABLE_LED_FRAME="true"
ENABLE_CONTAINER_WALL="true"

# -------------------- Lambert-return obstructions --------------------
LAMBERT_ENABLE_DOOR="false"
LAMBERT_ENABLE_CAMERA="false"
LAMBERT_ENABLE_DUCT="true"
LAMBERT_ENABLE_DUCT_STAIR="true"
LAMBERT_ENABLE_LED_FRAME="true"
LAMBERT_ENABLE_CONTAINER_WALL="true"

# -------------------- Files --------------------
SOURCE_CPP="ParallelLightTest.C"
RUN_CPP="ParallelLightTest_run.C"
EXECUTABLE="ParallelLightTest_run"

RESULT_DIR="mc_results_detector_body"
SEED_FILE="${RESULT_DIR}/random_seeds.txt"
SUMMARY_FILE="${RESULT_DIR}/summary.txt"
CONFIG_FILE="${RESULT_DIR}/run_config.txt"
LOG_FILE="${RESULT_DIR}/run.log"

# ============================================================
# Basic checks
# ============================================================

echo "============================================================"
echo "WFCTA Fast Efficiency Simulation"
echo "============================================================"

if [ ! -f "${SOURCE_CPP}" ]; then
    echo "ERROR: ${SOURCE_CPP} not found."
    exit 1
fi

for file in \
    src/parallel_light.cc \
    src/parallel_light_tracer.cc \
    src/diffuse_target.cc \
    src/detector.cc \
    src/wtelescope.cc
do
    if [ ! -f "${file}" ]; then
        echo "ERROR: ${file} not found."
        exit 1
    fi
done

for file in \
    include/parallel_light.h \
    include/parallel_light_tracer.h \
    include/diffuse_target.h \
    include/detector.h \
    include/wtelescope.h
do
    if [ ! -f "${file}" ]; then
        echo "ERROR: ${file} not found."
        exit 1
    fi
done

if ! command -v root-config >/dev/null 2>&1; then
    echo "ERROR: root-config not found."
    exit 1
fi

mkdir -p "${RESULT_DIR}"

# ============================================================
# Print configuration
# ============================================================

echo "Monte Carlo"
echo "  Total photons       : ${TOTAL_PHOTONS}"
echo "  Batch size          : ${BATCH_SIZE}"
echo "  Maximum workers     : ${MAX_WORKERS}"
echo "  Master seed         : ${MASTER_SEED}"

echo "Parallel light"
echo "  Position            : (${SOURCE_X}, ${SOURCE_Y}, ${SOURCE_Z}) mm"
echo "  Radius              : ${SOURCE_RADIUS} mm"
echo "  Wavelength          : ${WAVELENGTH} nm"

echo "Diffuse target"
echo "  Position            : (${TARGET_X}, ${TARGET_Y}, ${TARGET_Z}) mm"
echo "  Size                : ${TARGET_WIDTH} x ${TARGET_HEIGHT} mm"
echo "  Reflectivity        : ${TARGET_REFLECTIVITY}"

echo "CCD"
echo "  Center              : (${CCD_X}, ${CCD_Y}, ${CCD_Z}) mm"
echo "  LookAt              : (${CCD_LOOKAT_X}, ${CCD_LOOKAT_Y}, ${CCD_LOOKAT_Z}) mm"
echo "  Focal length        : ${CCD_FOCAL_LENGTH} mm"
echo "  F-number            : F/${CCD_F_NUMBER}"
echo "  FOV                 : ${CCD_HORIZONTAL_FOV} x ${CCD_VERTICAL_FOV} deg"
echo "  Disk diameter       : ${CCD_DISK_DIAMETER} mm"
echo "  Base                : ${CCD_BASE_WIDTH} x ${CCD_BASE_HEIGHT} mm"
echo "  Base left offset    : ${CCD_BASE_LEFT_OFFSET} mm"
echo "  Physical hole       : ${CCD_HOLE_DIAMETER} mm"
echo "  Hole edge distance  : ${CCD_HOLE_EDGE_DISTANCE} mm"

echo "Obstructions"
echo "  Door                : ${ENABLE_DOOR}"
echo "  Camera              : ${ENABLE_CAMERA}"
echo "  Duct                : ${ENABLE_DUCT}"
echo "  Duct stair          : ${ENABLE_DUCT_STAIR}"
echo "  LED frame           : ${ENABLE_LED_FRAME}"
echo "  Container wall      : ${ENABLE_CONTAINER_WALL}"

echo "Output"
echo "  Result directory    : ${RESULT_DIR}"
echo "============================================================"

# ============================================================
# Generate temporary C++ source
# ============================================================

cp "${SOURCE_CPP}" "${RUN_CPP}"

replace_line()
{
    local pattern="$1"
    local replacement="$2"

    if ! grep -Fq "${pattern}" "${RUN_CPP}"; then
        echo "ERROR: Cannot find configuration line:"
        echo "       ${pattern}"
        exit 1
    fi

    sed -i "s|^${pattern}.*|${replacement}|" "${RUN_CPP}"
}

# ============================================================
# Monte Carlo
# ============================================================

replace_line \
    "static const long long TOTAL_PHOTONS =" \
    "static const long long TOTAL_PHOTONS = ${TOTAL_PHOTONS};"

replace_line \
    "static const long long BATCH_SIZE =" \
    "static const long long BATCH_SIZE = ${BATCH_SIZE};"

replace_line \
    "static const int MAX_WORKERS =" \
    "static const int MAX_WORKERS = ${MAX_WORKERS};"

replace_line \
    "static const uint64_t MASTER_SEED =" \
    "static const uint64_t MASTER_SEED = ${MASTER_SEED};"

# ============================================================
# Parallel light
# ============================================================

replace_line \
    "static const double SOURCE_X =" \
    "static const double SOURCE_X = ${SOURCE_X};"

replace_line \
    "static const double SOURCE_Y =" \
    "static const double SOURCE_Y = ${SOURCE_Y};"

replace_line \
    "static const double SOURCE_Z =" \
    "static const double SOURCE_Z = ${SOURCE_Z};"

replace_line \
    "static const double SOURCE_RADIUS =" \
    "static const double SOURCE_RADIUS = ${SOURCE_RADIUS};"

replace_line \
    "static const double WAVELENGTH =" \
    "static const double WAVELENGTH = ${WAVELENGTH};"

# ============================================================
# Diffuse target
# ============================================================

replace_line \
    "static const double TARGET_X =" \
    "static const double TARGET_X = ${TARGET_X};"

replace_line \
    "static const double TARGET_Y =" \
    "static const double TARGET_Y = ${TARGET_Y};"

replace_line \
    "static const double TARGET_Z =" \
    "static const double TARGET_Z = ${TARGET_Z};"

replace_line \
    "static const double TARGET_WIDTH =" \
    "static const double TARGET_WIDTH = ${TARGET_WIDTH};"

replace_line \
    "static const double TARGET_HEIGHT =" \
    "static const double TARGET_HEIGHT = ${TARGET_HEIGHT};"

replace_line \
    "static const double TARGET_REFLECTIVITY =" \
    "static const double TARGET_REFLECTIVITY = ${TARGET_REFLECTIVITY};"

# ============================================================
# CCD position
# ============================================================

replace_line \
    "static const double CCD_X =" \
    "static const double CCD_X = ${CCD_X};"

replace_line \
    "static const double CCD_Y =" \
    "static const double CCD_Y = ${CCD_Y};"

replace_line \
    "static const double CCD_Z =" \
    "static const double CCD_Z = ${CCD_Z};"

# ============================================================
# CCD LookAt
# ============================================================

replace_line \
    "static const double CCD_LOOKAT_X =" \
    "static const double CCD_LOOKAT_X = ${CCD_LOOKAT_X};"

replace_line \
    "static const double CCD_LOOKAT_Y =" \
    "static const double CCD_LOOKAT_Y = ${CCD_LOOKAT_Y};"

replace_line \
    "static const double CCD_LOOKAT_Z =" \
    "static const double CCD_LOOKAT_Z = ${CCD_LOOKAT_Z};"

# ============================================================
# CCD optics
# ============================================================

replace_line \
    "static const double CCD_FOCAL_LENGTH =" \
    "static const double CCD_FOCAL_LENGTH = ${CCD_FOCAL_LENGTH};"

replace_line \
    "static const double CCD_F_NUMBER =" \
    "static const double CCD_F_NUMBER = ${CCD_F_NUMBER};"

replace_line \
    "static const double CCD_HORIZONTAL_FOV =" \
    "static const double CCD_HORIZONTAL_FOV = ${CCD_HORIZONTAL_FOV};"

replace_line \
    "static const double CCD_VERTICAL_FOV =" \
    "static const double CCD_VERTICAL_FOV = ${CCD_VERTICAL_FOV};"

# ============================================================
# CCD mechanical geometry
# ============================================================

replace_line \
    "static const double CCD_DISK_DIAMETER =" \
    "static const double CCD_DISK_DIAMETER = ${CCD_DISK_DIAMETER};"

replace_line \
    "static const double CCD_BASE_WIDTH =" \
    "static const double CCD_BASE_WIDTH = ${CCD_BASE_WIDTH};"

replace_line \
    "static const double CCD_BASE_HEIGHT =" \
    "static const double CCD_BASE_HEIGHT = ${CCD_BASE_HEIGHT};"

replace_line \
    "static const double CCD_BASE_LEFT_OFFSET =" \
    "static const double CCD_BASE_LEFT_OFFSET = ${CCD_BASE_LEFT_OFFSET};"

replace_line \
    "static const double CCD_HOLE_DIAMETER =" \
    "static const double CCD_HOLE_DIAMETER = ${CCD_HOLE_DIAMETER};"

replace_line \
    "static const double CCD_HOLE_EDGE_DISTANCE =" \
    "static const double CCD_HOLE_EDGE_DISTANCE = ${CCD_HOLE_EDGE_DISTANCE};"

# ============================================================
# Parallel-light obstruction switches
# ============================================================

replace_line \
    "static const bool ENABLE_DOOR =" \
    "static const bool ENABLE_DOOR = ${ENABLE_DOOR};"

replace_line \
    "static const bool ENABLE_CAMERA =" \
    "static const bool ENABLE_CAMERA = ${ENABLE_CAMERA};"

replace_line \
    "static const bool ENABLE_DUCT =" \
    "static const bool ENABLE_DUCT = ${ENABLE_DUCT};"

replace_line \
    "static const bool ENABLE_DUCT_STAIR =" \
    "static const bool ENABLE_DUCT_STAIR = ${ENABLE_DUCT_STAIR};"

replace_line \
    "static const bool ENABLE_LED_FRAME =" \
    "static const bool ENABLE_LED_FRAME = ${ENABLE_LED_FRAME};"

replace_line \
    "static const bool ENABLE_CONTAINER_WALL =" \
    "static const bool ENABLE_CONTAINER_WALL = ${ENABLE_CONTAINER_WALL};"

# ============================================================
# Lambert-return obstruction switches
# ============================================================

replace_line \
    "static const bool LAMBERT_ENABLE_DOOR =" \
    "static const bool LAMBERT_ENABLE_DOOR = ${LAMBERT_ENABLE_DOOR};"

replace_line \
    "static const bool LAMBERT_ENABLE_CAMERA =" \
    "static const bool LAMBERT_ENABLE_CAMERA = ${LAMBERT_ENABLE_CAMERA};"

replace_line \
    "static const bool LAMBERT_ENABLE_DUCT =" \
    "static const bool LAMBERT_ENABLE_DUCT = ${LAMBERT_ENABLE_DUCT};"

replace_line \
    "static const bool LAMBERT_ENABLE_DUCT_STAIR =" \
    "static const bool LAMBERT_ENABLE_DUCT_STAIR = ${LAMBERT_ENABLE_DUCT_STAIR};"

replace_line \
    "static const bool LAMBERT_ENABLE_LED_FRAME =" \
    "static const bool LAMBERT_ENABLE_LED_FRAME = ${LAMBERT_ENABLE_LED_FRAME};"

replace_line \
    "static const bool LAMBERT_ENABLE_CONTAINER_WALL =" \
    "static const bool LAMBERT_ENABLE_CONTAINER_WALL = ${LAMBERT_ENABLE_CONTAINER_WALL};"

# ============================================================
# Output paths
# ============================================================

replace_line \
    "static const char *RESULT_DIR =" \
    "static const char *RESULT_DIR = \"${RESULT_DIR}\";"

replace_line \
    "static const char *SEED_FILE =" \
    "static const char *SEED_FILE = \"${SEED_FILE}\";"

replace_line \
    "static const char *SUMMARY_FILE =" \
    "static const char *SUMMARY_FILE = \"${SUMMARY_FILE}\";"

# ============================================================
# Compile
# ============================================================

echo
echo "Compiling..."

g++ -O3 -std=c++11 \
    "${RUN_CPP}" \
    src/parallel_light.cc \
    src/parallel_light_tracer.cc \
    src/diffuse_target.cc \
    src/detector.cc \
    src/wtelescope.cc \
    -Iinclude \
    $(root-config --cflags --libs) \
    -o "${EXECUTABLE}"

echo "Compilation completed."
echo "Executable: ${EXECUTABLE}"

# ============================================================
# Save configuration
# ============================================================

cat > "${CONFIG_FILE}" << EOF
============================================================
WFCTA Monte Carlo Run Configuration
============================================================

[Monte Carlo]
TOTAL_PHOTONS=${TOTAL_PHOTONS}
BATCH_SIZE=${BATCH_SIZE}
MAX_WORKERS=${MAX_WORKERS}
MASTER_SEED=${MASTER_SEED}

[Parallel light]
SOURCE_X=${SOURCE_X}
SOURCE_Y=${SOURCE_Y}
SOURCE_Z=${SOURCE_Z}
SOURCE_RADIUS=${SOURCE_RADIUS}
WAVELENGTH=${WAVELENGTH}

[Diffuse target]
TARGET_X=${TARGET_X}
TARGET_Y=${TARGET_Y}
TARGET_Z=${TARGET_Z}
TARGET_WIDTH=${TARGET_WIDTH}
TARGET_HEIGHT=${TARGET_HEIGHT}
TARGET_REFLECTIVITY=${TARGET_REFLECTIVITY}

[CCD position]
CCD_X=${CCD_X}
CCD_Y=${CCD_Y}
CCD_Z=${CCD_Z}

[CCD LookAt]
CCD_LOOKAT_X=${CCD_LOOKAT_X}
CCD_LOOKAT_Y=${CCD_LOOKAT_Y}
CCD_LOOKAT_Z=${CCD_LOOKAT_Z}

[CCD optics]
CCD_FOCAL_LENGTH=${CCD_FOCAL_LENGTH}
CCD_F_NUMBER=${CCD_F_NUMBER}
CCD_HORIZONTAL_FOV=${CCD_HORIZONTAL_FOV}
CCD_VERTICAL_FOV=${CCD_VERTICAL_FOV}
CCD_EFFECTIVE_APERTURE=$(awk "BEGIN {printf \"%.9f\", ${CCD_FOCAL_LENGTH}/${CCD_F_NUMBER}}")

[CCD mechanical geometry]
CCD_DISK_DIAMETER=${CCD_DISK_DIAMETER}
CCD_BASE_WIDTH=${CCD_BASE_WIDTH}
CCD_BASE_HEIGHT=${CCD_BASE_HEIGHT}
CCD_BASE_LEFT_OFFSET=${CCD_BASE_LEFT_OFFSET}
CCD_HOLE_DIAMETER=${CCD_HOLE_DIAMETER}
CCD_HOLE_EDGE_DISTANCE=${CCD_HOLE_EDGE_DISTANCE}
CCD_HOLE_CENTER_X=$(awk "BEGIN {printf \"%.9f\", -${CCD_DISK_DIAMETER}/2.0 + ${CCD_HOLE_EDGE_DISTANCE} + ${CCD_HOLE_DIAMETER}/2.0}")
CCD_HOLE_CENTER_Y=0.0

[Parallel-light obstructions]
ENABLE_DOOR=${ENABLE_DOOR}
ENABLE_CAMERA=${ENABLE_CAMERA}
ENABLE_DUCT=${ENABLE_DUCT}
ENABLE_DUCT_STAIR=${ENABLE_DUCT_STAIR}
ENABLE_LED_FRAME=${ENABLE_LED_FRAME}
ENABLE_CONTAINER_WALL=${ENABLE_CONTAINER_WALL}

[Lambert-return obstructions]
LAMBERT_ENABLE_DOOR=${LAMBERT_ENABLE_DOOR}
LAMBERT_ENABLE_CAMERA=${LAMBERT_ENABLE_CAMERA}
LAMBERT_ENABLE_DUCT=${LAMBERT_ENABLE_DUCT}
LAMBERT_ENABLE_DUCT_STAIR=${LAMBERT_ENABLE_DUCT_STAIR}
LAMBERT_ENABLE_LED_FRAME=${LAMBERT_ENABLE_LED_FRAME}
LAMBERT_ENABLE_CONTAINER_WALL=${LAMBERT_ENABLE_CONTAINER_WALL}

[Output]
RESULT_DIR=${RESULT_DIR}
SUMMARY_FILE=${SUMMARY_FILE}
SEED_FILE=${SEED_FILE}
LOG_FILE=${LOG_FILE}
============================================================
EOF

echo "Configuration saved: ${CONFIG_FILE}"

# ============================================================
# Run
# ============================================================

echo
echo "============================================================"
echo "Starting simulation"
echo "============================================================"

"./${EXECUTABLE}" 2>&1 | tee "${LOG_FILE}"

echo
echo "============================================================"
echo "Simulation finished"
echo "============================================================"
echo "Summary : ${SUMMARY_FILE}"
echo "Seeds   : ${SEED_FILE}"
echo "Config  : ${CONFIG_FILE}"
echo "Log     : ${LOG_FILE}"
echo "============================================================"
