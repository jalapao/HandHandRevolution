// Copyright (C) 2013-2014 Thalmic Labs Inc.
// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <time.h>
#include <thread>
#include <unistd.h>

// The only file that needs to be included to use the Myo C++ SDK is myo.hpp.
#include <myo/myo.hpp>

#define screenHeight 40
#define timeToGester 12
int counter = 0;
int score = 0;
int gesterGenerated = -1;
std::string screen[screenHeight];
std::string poseString;
int expectedGester[screenHeight];
int streak = 0;
int lifePoint = 10;
int actualGester = -2;

myo::Myo* myop;
myo::Hub hub("com.example.hello-myo");

using std::cout;
using std::endl;

// Classes that inherit from myo::DeviceListener can be used to receive events from Myo devices. DeviceListener
// provides several virtual functions for handling different kinds of events. If you do not override an event, the
// default behavior is to do nothing.
class DataCollector : public myo::DeviceListener {
public:
    DataCollector()
    : onArm(false), isUnlocked(false), roll_w(0), pitch_w(0), yaw_w(0), currentPose(), num_drops(0)
    {
    }

    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        roll_w = 0;
        pitch_w = 0;
        yaw_w = 0;
        onArm = false;
        isUnlocked = false;
    }

    // onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
    // as a unit quaternion.
    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
    {
        using std::atan2;
        using std::asin;
        using std::sqrt;
        using std::max;
        using std::min;

        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
                           1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
        float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
        float yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
                        1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));

        // Convert the floating point angles in radians to a scale from 0 to 18.
        roll_w = static_cast<int>((roll + (float)M_PI)/(M_PI * 2.0f) * 18);
        pitch_w = static_cast<int>((pitch + (float)M_PI/2.0f)/M_PI * 18);
        yaw_w = static_cast<int>((yaw + (float)M_PI)/(M_PI * 2.0f) * 18);
    }

    // onPose() is called whenever the Myo detects that the person wearing it has changed their pose, for example,
    // making a fist, or not making a fist anymore.
    void onPose(myo::Myo* myo, uint64_t timestamp, myo::Pose pose)
    {
        currentPose = pose;

        if (pose != myo::Pose::unknown && pose != myo::Pose::rest) {
            // Tell the Myo to stay unlocked until told otherwise. We do that here so you can hold the poses without the
            // Myo becoming locked.
            myo->unlock(myo::Myo::unlockHold);

            // Notify the Myo that the pose has resulted in an action, in this case changing
            // the text on the screen. The Myo will vibrate.
            myo->notifyUserAction();
        } else {
            // Tell the Myo to stay unlocked only for a short period. This allows the Myo to stay unlocked while poses
            // are being performed, but lock after inactivity.
            myo->unlock(myo::Myo::unlockTimed);
        }
    }

    // onArmSync() is called whenever Myo has recognized a Sync Gesture after someone has put it on their
    // arm. This lets Myo know which arm it's on and which way it's facing.
    void onArmSync(myo::Myo* myo, uint64_t timestamp, myo::Arm arm, myo::XDirection xDirection)
    {
        onArm = true;
        whichArm = arm;
    }

    // onArmUnsync() is called whenever Myo has detected that it was moved from a stable position on a person's arm after
    // it recognized the arm. Typically this happens when someone takes Myo off of their arm, but it can also happen
    // when Myo is moved around on the arm.
    void onArmUnsync(myo::Myo* myo, uint64_t timestamp)
    {
        onArm = false;
    }

    // onUnlock() is called whenever Myo has become unlocked, and will start delivering pose events.
    void onUnlock(myo::Myo* myo, uint64_t timestamp)
    {
        isUnlocked = true;
    }

    // onLock() is called whenever Myo has become locked. No pose events will be sent until the Myo is unlocked again.
    void onLock(myo::Myo* myo, uint64_t timestamp)
    {
        isUnlocked = false;
    }

    // There are other virtual functions in DeviceListener that we could override here, like onAccelerometerData().
    // For this example, the functions overridden above are sufficient.

    // We define this function to print the current values that were updated by the on...() functions above.

    std::string getCurrentPose() {
        return currentPose.toString();
    }

    void print()
    {
        system("clear");

        srand(time(0)); // use a different random seed every time.

        num_drops++;

        // Clear the current line


        /*
        // Print out the orientation. Orientation data is always available, even if no arm is currently recognized.
        std::cout << '[' << std::string(roll_w, '*') << std::string(18 - roll_w, ' ') << ']'
                  << '[' << std::string(pitch_w, '*') << std::string(18 - pitch_w, ' ') << ']'
                  << '[' << std::string(yaw_w, '*') << std::string(18 - yaw_w, ' ') << ']';
         */

        if (onArm) {
            std::cout << '\r';
            // Print out the lock state, the currently recognized pose, and which arm Myo is being worn on.

            // Pose::toString() provides the human-readable name of a pose. We can also output a Pose directly to an
            // output stream (e.g. std::cout << currentPose;). In this case we want to get the pose name's length so
            // that we can fill the rest of the field with spaces below, so we obtain it as a string using toString().
            //std::cout << "gesterGenerated = " << gesterGenerated;
            for (int lineIndex = screenHeight - 2; lineIndex > 0; --lineIndex) {
                screen[lineIndex] = screen[lineIndex - 1];
            }
            for (int lineIndex = screenHeight - 1; lineIndex > 0; --lineIndex) {
                expectedGester[lineIndex] = expectedGester[lineIndex - 1];
            }
            screen[0] = "";
            expectedGester[0] = -1;
            if (counter % timeToGester == 0) {
                gesterGenerated = (rand() % 5);
                expectedGester[0] = gesterGenerated;
                switch (gesterGenerated) {
                    //case 0: screen[0] = "rest"; // We'll just pretend as if REST never appears.
                    case 0: screen[0] = "     ";
                        break;
                    case 1: screen[0] = "         \\|||/     ";
                        break;
                    case 2: screen[0] = "                       <<----";
                        break;
                    case 3: screen[0] = "                                 ---->>";
                        break;
                    case 4: screen[0] = "                                            O ";
                        break;
                }
            }

            if (num_drops >= screenHeight) {
                // Only do important score changes when touch the first one
                if (actualGester == expectedGester[screenHeight - 1]) {
                    score++;
                    if (streak < 0) {
                        streak = 0;
                    }
                    streak++;
                } else if (expectedGester[screenHeight - 1] >= 0) {
                    // FIXME track here about when to lose
                    streak--;

                    if (expectedGester[screenHeight - 1] > 0) {
                        lifePoint--;
                    }
                }
            }

            for (int screenIndex = 0; screenIndex < screenHeight - 1; ++screenIndex) {
                std::cout << screen[screenIndex] << std::endl;
                //std::cout << "in for loop in if onArm";
            }
            std::cout << screen[screenHeight - 1];
            // std::cout << (isUnlocked ? ";unlocked" : ";locked  ");
            std::cout << "\t  Score: " << score << "; Your Input: " << poseString << "; Life: " << lifePoint << std::endl;
            /*
            std::cout << '[' << (isUnlocked ? "unlocked" : "locked  ") << ']'
                      //<< '[' << (whichArm == myo::armLeft ? "L" : "R") << ']'
                      << '[' << poseString << std::string(14 - poseString.size(), ' ') << ']';
             */
             //std::cout << "in if onArm";
            std::cout << std::flush;
        }
        /*
        else {
            // Print out a placeholder for the arm and pose when Myo doesn't currently know which arm it's on.
            std::cout << '[' << std::string(8, ' ') << ']' << "[?]" << '[' << std::string(14, ' ') << ']';
        } */

    }

    // These values are set by onArmSync() and onArmUnsync() above.
    bool onArm;
    myo::Arm whichArm;

    // This is set by onUnlocked() and onLocked() above.
    bool isUnlocked;

    // These values are set by onOrientationData() and onPose() above.
    int roll_w, pitch_w, yaw_w;
    myo::Pose currentPose;

    // Controller variables for the game
    unsigned int num_drops; // the number of drops already
};

DataCollector collector;

void gestureFunc() {

    while (1) {
        myop->unlock(myo::Myo::unlockHold);
        hub.run(1000/10);

        poseString = collector.getCurrentPose();

        if (poseString.compare("rest") == 0) {
            actualGester = 0;
        } else if (poseString.compare("fingersSpread") == 0) {
            actualGester = 1;
        } else if (poseString.compare("waveIn") == 0) {
            actualGester = 2;
        } else if (poseString.compare("waveOut") == 0) {
            actualGester = 3;
        } else if (poseString.compare("fist") == 0) {
            actualGester = 4;
        }
    }
}

void gameLogic() {

    counter = 0;
    score = 0;
    gesterGenerated = -1;

    streak = 0;
    lifePoint = 10;

    for (int i = 0; i != screenHeight; i++) {
        expectedGester[i] = -1;
        screen[i] = "";
    }

    screen[screenHeight-1] = "====   fingersSpread | waveIn | waveOut | fist ====";

    while (1) {

        collector.print();
        counter++;

        if (lifePoint <= 0) {
            std::cout << "Game Over" << std::endl;
            break;
        }

        usleep(100000);
    }
}

const int START = 1;
const int END = 2;

int main(int argc, char** argv) {
    try {

        // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
        // publishing your application. The Hub provides access to one or more Myos.
        std::cout << "Attempting to find a Myo..." << std::endl;

        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        // waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
        // if that fails, the function will return a null pointer.
        myop = hub.waitForMyo(10000);

        // If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myop) {
            throw std::runtime_error("Unable to find a Myo!");
        }

        // We've found a Myo.
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;

        // Next we construct an instance of our DeviceListener, so that we can register it with the Hub.

        // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
        // Hub::run() to send events to all registered device listeners.
        hub.addListener(&collector);
        myop->unlock(myo::Myo::unlockHold);
        //std::cout << screen[screenHeight-1];
        // Finally we enter our main loop.

        //screen[screenHeight-1] = "======================================";

        // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
    }

    std::thread gestureThread(gestureFunc);

    while (true) {
        cout << "******* MENU *******" << endl;
        cout << "****  1  Start  ****" << endl;
        cout << "****  2  End    ****" << endl;

        int mode;

        std::cin >> mode;

        switch (mode) {
            case START:
                gameLogic();
                break;
            case END:
                return 0;
            default:
                continue;
        }
    }
}
