Information in this file is pretty sparse at the moment, hopefully
it'll grow with time.

For information about the compression and scrambling see the file named
HACKING.txt.

Block 0
-------

This block contains information about dynamic objects; ships, ports, planets,
etc. It begins at file offset 0x11 with 0x73 bytes, each describing the flags
for a specific object. Then follows a byte of unknown use. Then follows the
real data, 0x73 blocks each 0x152 bytes long, each describing one object.

For information on the flags see "PhysObj Flags" in srctools/plstate.txt in
the JJFFE distribution (http://www.jaj22.demon.co.uk/). For information on
the structure of the objects see "PhysObj struct" in the same file.


Block 1
-------

<JohnJ> It's a list containing 8 bytes of information for each code module.

I have no idea what that means, but there you go. Begins at file offset 0x986C
and comes out at 0x100 bytes on every saved game I've tested it with.


Block 2
-------

No idea what this one is for either. The smallest block at just 0x40 bytes.
Begins at file offset 0x997D.


Block 3
-------

Probably the most interesting block. Begins at file offset 0x99CE and consists
of 0x556a bytes of global state information; date, time, credits, cargo,
ranking/rating/title, etc.

See srctools/gmstruct.txt in the JJFFE distribution for details.


Block 4
-------

A particularly uninteresting block of 0x51 bytes beginning at file offset
0xEF49. Contains 0x11 bytes of the null-terminated textual name of the system
selected, followed by 0x40 bytes of the null-terminated textual description
of the type of that system (e.g. "Bright giant star").

Following block 4 there is one padding byte copied from the scrambled saved
game (if FFE writes one, 0x0 is copied otherwise). The padding byte is
irrelevant to FFE when it's loading game, but we copy it so the unmodied
output of unmangle, fed into mangle, will yield an exact copy of the
original save.


Block 5
-------

This block is actually the first block in a saved game. It's moved to the end
by unmangle because of its variable length. It begins at file offset 0xEFAC
and continues to the end of the file. It contains information about the
hand-coded missions and the news papers, but I've no idea on the details.


