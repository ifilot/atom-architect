Atom Architect
==============

Atom Architect is a tool for loading, visualizing, and analyzing atomistic
structures from electronic structure calculations performed in VASP.

Philosophy
----------

Atom Architect is built around a simple, but fundamental idea:

**There is no “up” in space.**

In chemistry and physics, atomic systems are inherently **translationally and rotationally invariant**.  
A molecule does not become a different molecule because it is shifted, rotated, or placed at another
point in space. Any coordinate system we impose is therefore **arbitrary** — useful for computation,
but meaningless from a physical perspective.

Traditional modeling tools often require users to position atoms or fragments by entering
absolute Cartesian coordinates with respect to a fixed origin. While mathematically valid, this
approach forces users to think in terms of an artificial reference frame rather than in terms of
chemical structure.

Atom Architect takes a different approach.

Instead of asking *“Where is this atom in space?”*, Atom Architect asks:

*“How is this fragment positioned relative to the atoms that already exist?”*

Fragments are positioned by **selecting atoms**, typically chemically relevant sites such as
metal atoms on a catalyst surface. Rotations are performed around **axes defined by existing atoms**,
not around abstract coordinate directions. In this way, every transformation is expressed in terms
of chemically meaningful relationships.

This design has two important consequences:

* Structures remain intuitive and reproducible, independent of their absolute orientation.
* User interactions closely mirror how chemists think about bonding, coordination, and geometry.

By eliminating the notion of a privileged origin or direction, Atom Architect keeps the focus on
**chemistry**, not coordinates.
