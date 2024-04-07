# Implementing Spatial Interactions with Third-Party Accessories

Establish a connection with a nearby accessory to begin periodically receiving its distance from the user's device.

## Overview

- This base project is associated with WWDC21 session [10165: Explore Nearby Interaction with Third-Party Accessories](https://developer.apple.com/wwdc21/10165).
- If using iOS 16 or later, it enables Camera Assistance, associated with WWDC22 session [10008: What's new in Nearby Interaction](https://developer.apple.com/wwdc22/10008).

## Configure the Sample Code Project

Build using XCode and run on an iPhone running iOS 15 or later, that contains a U1 chip. The sample app interacts with an accessory you partner with or develop using the [Ultra Wideband (UWB) third-party device specification](https://developer.apple.com/nearby-interaction).

Download XCode 14 from:
[Apple Mac App Store: XCode 14](https://apps.apple.com/us/app/xcode/id497799835?mt=12)

On Xcode, click on File->Open... navigate to the Qorvo NI project folder and open the NINearbyAccessorySample.xcodeproj file.

In the menu on the left, select the project file (click on the project name, NINearbyAccessorySample), then click on the "Signing & Capabilities" tab and set value for Team.

Connect to the macOS device an iPhone equipped with a U1 chip:
- iPhone 11 (11, Pro and Pro Max)
- iPhone 12 (12, Mini, Pro and Pro Max)
- iPhone 13 (13, Mini, Pro and Pro Max)
- iPhone 14 (14, Mini, Pro and Pro Max)

**The iPhone must be updated to the iOS 15 or later.**

Select the device (iPhone) on the top navigation bar "NINearbyAccessorySample>(your iOS Device)" and click on the play icon button on the left.
The app will be built and deployed to the iPhone.
