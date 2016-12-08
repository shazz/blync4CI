# blync4CI

A little REST interface to control blync lights, with makefile for OpenWRT.
This code is reusing Blynux (https://github.com/ticapix/blynux) and the tiny HTTP server from Bhijeet Rastogi (http://www.google.com/profiles/abhijeet.1989). Thanks to them.

*Disclaimer*: This is some very old code I quickly wrote some years ago (2014 ?) and published to github on request of people really using it :)


## Typical usage: augmented Continuous Integration

Make your favorite CI engine (Jenkins,...) change the color of your Blync lights depending on the job status!

* connect one or multiple Blync lights (https://www.embrava.com/products/blync-light) to an OpenWRT mifi router (usually the so good TP-Link TL-WR703N or TL-MR3020) which will expose a simple REST interface thanks to this program.
* Then, configure your CI server to call the REST interface for success (green) or failed (red) builds

So quite useless but funny, you can't hide your failed build :)


## Main features

* runs on port 10000 by default
* discovers multiple blync lights connected on USB (you may use a Hub) and assigns them a number automatically (<device_id>, starts at 0)
* light control with HTTP GET commands on URL `http://<host>:10000/<device_id>/color/<chosen_color>`, with `<chosen_color>=<green|red|blue|cyan|yellow|magenta|white|off>`
* *WARNING* : server can be shutdown using `http://<host>:10000/state/shutdown` (but you cant restart it without logging in to the device, see below)



## Compilation and Installation steps

First compile blynux:

```bash
> make blynux
```

Then you may wish to optimize the executable to reduce its size on target

```bash
> rm blynuxCompressed
> upx --ultra-brute -o blynuxCompressed blynux
```

Finally deploy on target

```bash
> scp blynuxCompressed root@<BlynkerDeviceHostName>:/usr/bin/blynux
> scp ./init.d-script/blynuxD root@<BlynkerDeviceHostName>:/etc/init.d/blynuxD
```

You may now get the status / start / stop the server on target using the following commands:
```bash
> ssh root@<BlynkerDeviceHostName>
> /etc/init.d/blynuxD status
> /etc/init.d/blynuxD start
> /etc/init.d/blynuxD stop
```