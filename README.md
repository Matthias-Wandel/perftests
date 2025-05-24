# perftests
Simple performance tests for looking into how different CPUs differ in approach.
<p>
the "perftests" program, compiled in the "tests" directory can run a sequence
of tests and / or repeat tests.  Priority can be left as default, or background
or high priority windows tasks.
<p>
Multiple treads of running tests can be
launched to run on specific cores using thead affinity.  This is useful for
testing how modern CPUs power manage under varying loads on efficiency cores
and performance cores.
<p>
These are a companion to my video
<a href="https://youtu.be/m7PVZixO35c">New computers don't speed up old code</a>
<p>
This program also compiles on linux.
