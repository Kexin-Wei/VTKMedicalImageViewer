#pragma once
#include "unit.h"
#include <QList>
#include <QString>

// hardwareCenter
const int RELAY_WAITTIME = 2000;
const int AMPLIFIER_SERIALCHECK_BUFFERTIME = 2000;
const int AMPLIFIER_SERIALON_MAXIMUM_WAITTIME = 10000;
const int STORK_ON_BUFFERTIME = 1000;
const int STORK_STREAMSERVER_WAITTIME = 4000;
const int STORK_ON_EXPECTEDTIME = 30000;
const int STORK_ON_MAXIMUM_WAITTIME = 120000;

// database
const QString DATABASE_FILE_SUFFIX = "_db";
const QString PATIENT_RECOVERY = "recovery.ptnt";
const QString SURGERY_RECOVERY = "recovery.srgy";

// robot3prs
const int ROBOT3PRS_TIMEOUT = 5000;
const bool ROBOT3PRS_DIRECTCONNECTION = false;
const bool ROBOT3PRS_AUTOINIT = false;
const int ROBOT3PRS_MESSAGETIMER = 6;
const int ROBOT3PRS_SENSORTIMER = 200;
const int ROBOT3PRS_WATCHDOGTIMER = 4000;
const int ROBOT3PRS_OFFSETX = 5;

// video
const ultrast::infrastructure::utility::unit::Seconds MAX_RECORDING_TIME = 30;

// registration
const int REGISTRATION_DURATION = 10;

// text size
const int IMAGEVIEWER_TEXT_FONT = 18;
const int IMAGEVIEWER_NUMBER_FONT = 18;

// probe control
const int PROBEWORKER_TIMEINTERVAL = 80;

// prostate color
const QString PROSTATE_COLOR = "#80F0B8A0"; // AARRGGBB
const QString LESION_COLOR = "#B3FFC000"; // AARRGGBB

// disk manager
const ultrast::infrastructure::utility::unit::GB MINIMAL_DISK_SPACE = 20;

// image viewer
const int IMAGE_SCENE_WIDTH = 1862;
const int IMAGE_SCENE_HEIGHT = 932;

// extract surface parameters
const double RELAX_FACTOR = 0.3;
const double RESIZE_RATIO = 2.0;
const int SMOOTH_ITERATION = 10;

//thumbnail viewer
const int THUMBNAIL_WIDTH = 175;
const int THUMBNAIL_HEIGHT = 120;

// support import file extension
const QList<QString> SUPPORTED_FORMAT = { "nrrd", "mhd", "mha", "dcm", "nii.gz", "nii" };

//viewer default view angle
const double STEREOVIEWER_DEFAULT_VIEW_ANGLE = 30;
