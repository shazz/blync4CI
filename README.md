# blync4CI
A little REST interface to use blync lights for CI engines thru an OpenWRT mifi router.

In brief, you can connect one or multiple Blync lights (https://www.embrava.com/products/blync-light) to an OpenWRT mifi router (usually the so good TP-Link TL-WR703N or TL-MR3020) which will expose a simple REST interface.

Then, using your favorite CI engine (Jenkins,...), simply call the REST interface for success (green) or failed (red) build.

This code is reusing Blynux (https://github.com/ticapix/blynux) and soem code from Bhijeet Rastogi (http://www.google.com/profiles/abhijeet.1989). Thanks them.

*Disclaimer*: This is some very old code I quickly throw some years ago (2014 ?) and published to github on request of people really using it :)

