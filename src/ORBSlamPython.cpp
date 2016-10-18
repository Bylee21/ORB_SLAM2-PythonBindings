#include <opencv2/core/core.hpp>
#include <ORB_SLAM2/KeyFrame.h>
#include <ORB_SLAM2/Converter.h>
#include "ORBSlamPython.h"

BOOST_PYTHON_MODULE(orbslam2)
{
    boost::python::class_<ORBSlamPython, boost::noncopyable>("system", boost::python::init<std::string, std::string>())
        .def("initialize", &ORBSlamPython::initialize)
        .def("loadAndProcessImage", &ORBSlamPython::loadAndProcessImage)
        .def("shutdown", &ORBSlamPython::shutdown)
        .def("getTrajectoryPoints", &ORBSlamPython::getTrajectoryPoints);
}

ORBSlamPython::ORBSlamPython(std::string vocabFile, std::string settingsFile)
    : vocabluaryFile(vocabFile),
    settingsFile(settingsFile),
    system(std::make_shared<ORB_SLAM2::System>(ORB_SLAM2::System::MONOCULAR))
{
    
}

ORBSlamPython::~ORBSlamPython()
{
}

void ORBSlamPython::initialize()
{
    system->StartUp(vocabluaryFile, settingsFile, false);
}

void ORBSlamPython::loadAndProcessImage(std::string imageFile, double timestamp)
{
    cv::Mat im = cv::imread(imageFile, CV_LOAD_IMAGE_UNCHANGED);
    system->TrackMonocular(im, timestamp);
}

void ORBSlamPython::shutdown()
{
    system->Shutdown();
}

boost::python::list ORBSlamPython::getTrajectoryPoints() const
{
    // This is copied from the ORB_SLAM2 System.SaveTrajectoryTUM function, with some changes to output a python tuple.
    vector<std::shared_ptr<ORB_SLAM2::KeyFrame>> vpKFs = system->GetKeyFrames();
    std::sort(vpKFs.begin(), vpKFs.end(), ORB_SLAM2::KeyFrame::lId);

    // Transform all keyframes so that the first keyframe is at the origin.
    // After a loop closure the first keyframe might not be at the origin.
    //cv::Mat Two = vpKFs[0]->GetPoseInverse();

    boost::python::list trajectory;

    for(size_t i=0; i<vpKFs.size(); i++)
    {
        std::shared_ptr<ORB_SLAM2::KeyFrame> pKF = vpKFs[i];

       // pKF->SetPose(pKF->GetPose()*Two);

        if(pKF->isBad())
            continue;

        cv::Mat R = pKF->GetRotation().t();
        vector<float> q = ORB_SLAM2::Converter::toQuaternion(R);
        cv::Mat t = pKF->GetCameraCenter();
        
        trajectory.append(boost::python::make_tuple(
            pKF->mTimeStamp,    // Timestamp
            t.at<float>(0), t.at<float>(1), t.at<float>(2),  // Location
            q[0], q[1], q[2], q[3]  // Orientation
        ));

    }

    return trajectory;
}

