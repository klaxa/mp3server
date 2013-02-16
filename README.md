mp3server
=========

This is a low-load mp3 re-streaming server written in C.
That's about what it is.

Usage
-----

As of now there are no command line options, all it does at the moment, is open a listening socket on port 8080 and take the first client that connects to that socket as a source for an mp3-stream. Every other client connecting after the first one is considered a normal client, i.e. the stream is being sent to them. IPv6 is supported (thanks Sean_McG), if you have IPv6 disabled in your kernel, change the #define IPv6 1 to #define IPv6 0 in server.c.

Why would you do that if there are already things like icecast?
---------------------------------------------------------------

Yes indeed, there is software like icecast and it's pretty cool, however, it's also pretty heavy and I remember running icecast on a rather small server, it consumed quite a lot of resources, therefore I thought I'd write my own server.
Also coding is fun and I had *a lot* of fun coding this project until now.

But there's also mpd which is supposed to use less resources!
-------------------------------------------------------------

Yes indeed, it does, yet I was too lazy to figure out how to stream my music to an mpd instance on my server, I would have had to upload all my music to it to stream with mpd.

More reasons I did this and a bit history about this project
------------------------------------------------------------

Once upon a time in #anime on freenode, I was again posting the music I was
listening to, because it became a habit of mine to do so.

> \*	klaxa is listening to millie - Dreaming Forest
> \<Sean_McG\>    klaxa: you play good music -- do you stream it at all?
> \<klaxa\>	    no, should i?
> \*	Sean_McG would listen to it
> \<klaxa\>       it wouldn't be up all the time, but i guess i can set something up

So I thought up a few ways to do this...

> \<klaxa\>	i have mpd running, i'd try to add some output that i grab locally via http, have something connect to my server which then encodes to mulitple formats and streams
> \<klaxa\>	the hardest part i see right now is the streaming part
> \<klaxa\>	the rest shouldn't be too hard, i could even do it with netcat
> \<klaxa\>	if i knew how roboust mp3 is against starting to stream from just anywhere in the bytestream i could write my own server

I found out pretty soon that mp3 isn't roboust against "starting to stream from just anywhere in the bytestream", but that didn't demotivate me. It was actually the opposite
and I started reading the mp3 header specification.

I implemented the specification and pretty much finished up everything within about a week.

> \<@albel727\>   why restream mpd at all?
> \<klaxa\>       because... why not?
> \<klaxa\>       sure it can stream itself via httpd
> \<@albel727\>   ^
> \<klaxa\>       but for that i'd have to forward ports locally
> \<klaxa\>       and i'm too lazy to do that
> \<klaxa\>       so instead, i write code for a week

And that's basically why I wrote this.

How does it internally work?
----------------------------

(This might go further into detail than you actually care)

Okay how does mp3server work internally? It's actually rather simple I think:

First of all a server-socket is created on port 8080 and bound to the INADDR_ANY. The next step is to accept exactly one client, which will serve as the source-client, i.e. the client that sends an mp3-stream to the server. Now the interesting part beings. Before creating the server-socket, we actually did other stuff, we allocated a circular list for server-side buffering. This and using select() to check for writable clients prevents the network throughput from having huge spikes (http://klaxa.in/network\_spikes.png top left terminal). With buffering and select() we get a rather smooth network throughput graph (http://klaxa.in/no\_network\_spikes.png top right terminal). I implemented a global buffer as a circular list, because I wanted high efficiency and low resource waste. I looked at how mpd managed its http clients and found out that they keep a buffer for each client and run some sort of multi-threaded model to send the data to the client. I didn't want buffering per client and multi-threading felt evil and overkill. According to htop mp3server needs less than 4 kB of RAM and 0% CPU (at least on my low end kvm box, it actually needs a bit more on my android phone http://klaxa.in/mp3server\_android.png). So in the end I managed to write rather low-load code that even runs acceptably well on a phone (however, a rather overpowered one, test it yourself as much as you please). I tested it against memory leaks with valgrind too, and it appears there are no memory leaks present.

TODO
----

- Add Icy-Metadata support
- Add multiple re-stream encodings (other bitrate mp3, vorbis, opus)
- Add multiple input-stream decodings (flac, vorbis, opus)
