/*
 *  genome.h
 *  openc2e
 *
 *  Created by Alyssa Milburn on Sat 13 Nov 2004.
 *  Copyright (c) 2004-2006 Alyssa Milburn. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 */
#ifndef _C2E_GENOME_H
#define _C2E_GENOME_H

#include "endianlove.h"
#include "exceptions.h"
#include "lifestage.h"

#include <vector>
#include <string>

using std::ostream;
using std::istream;
using std::vector;

class gene;
struct geneNote;
class organGene;
	
//! A class for handling Creatures genome (.gen) files.
class genomeFile {
  uint8 cversion;
  organGene *currorgan;

  gene *nextGene(istream &); // returns NULL upon 'gend'
  geneNote *findNote(uint8 type, uint8 subtype, uint8 which);

  friend ostream &operator << (ostream &, const genomeFile &);
  friend istream &operator >> (istream &, genomeFile &);

public:
  vector<gene *> genes;

  void readNotes(istream &);
  void writeNotes(ostream &) const;

  uint8 getVersion() { return cversion; }

  gene *getGene(uint8 type, uint8 subtype, unsigned int seq);
};

struct geneNote {
	std::string description;
	std::string comments;
	uint8 which;
};

//! header flags of a gene
struct geneFlags {
  bool _mutable; // 'mutable' is a reserved keyword
  bool dupable;
  bool delable;
  bool maleonly;
  bool femaleonly;
  bool notexpressed;
  bool reserved1;
  bool reserved2;
  
  uint8 operator () () const;
  void operator () (uint8);

  geneFlags() { _mutable = true; dupable = true; delable = true; maleonly = false; femaleonly = false; notexpressed = false; reserved1 = false; reserved2 = false; }
};

//! header of a gene
struct geneHeader {
  geneFlags flags;

  uint8 generation;
  lifestage switchontime;
  uint8 mutweighting; // c2/c2e only
  uint8 variant; // c2e only

  // todo: are these values 'correct' for default values? - Fuzzie
  geneHeader() { generation = 0; switchontime = baby; mutweighting = 255; variant = 0; }
};

/*
  nextGene does the 'dirty work' of working out which class to create, and then calls
  the << operator as appropriate

  however the >> operator must do this work itself ('gene' and the type/subtype)
*/

//! The base class for all Creatures genes.
class gene {
  friend ostream &operator << (ostream &, const gene &);
  friend istream &operator >> (istream &, gene &);

protected:
  uint8 cversion;

  virtual uint8 type() const = 0;
  virtual uint8 subtype() const = 0;

  virtual void write(ostream &) const = 0;
  virtual void read(istream &) = 0;

  friend class genomeFile;

public:
  geneNote note;
  geneHeader header;

  gene(uint8 v) : cversion(v) { }
  virtual ~gene() { }

  uint8 version() { return cversion; }

  virtual const char *name() = 0;
  virtual const char *typeName() = 0;
};
 
//! gene: organ, either brain or normal
class organGene : public gene {
  bool brainorgan;

protected:
  uint8 type() const { if (brainorgan) return 0; else return 3; }
  uint8 subtype() const { if (brainorgan) return 1; else return 0; }

  void write(ostream &) const;
  void read(istream &);

public:
  vector<gene *> genes;

  bool isBrain() { return brainorgan; }

  uint8 clockrate;
  uint8 damagerate;
  uint8 lifeforce;
  uint8 biotickstart;
  uint8 atpdamagecoefficient;

  organGene(uint8 v, bool b) : gene(v), brainorgan(b) { }
  const char *name() { return "Organ"; }
  const char *typeName() { if (brainorgan) return "Brain"; return "Biochemistry"; }
};

//! gene: brain-type base class (not including brain organ)
class brainGene : public gene {
protected:
  uint8 type() const { return 0; }

  brainGene(uint8 v) : gene(v) { }
  const char *typeName() { return "Brain"; }
};

//! gene: c2e brain lobe
class c2eBrainLobeGene : public brainGene {
protected:
  uint8 subtype() const { return 0; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 id[4]; // todo: string
  uint16 updatetime; // 0 for never
  uint16 x;
  uint16 y;
  uint8 width;
  uint8 height;
  uint8 red;
  uint8 green;
  uint8 blue;
  uint8 WTA; // unused in final game?
  uint8 tissue;
  uint8 initrulealways;
  uint8 spare[7];
  uint8 initialiserule[48];
  uint8 updaterule[48];

  c2eBrainLobeGene(uint8 v) : brainGene(v) { }
  const char *name() { return "Lobe"; }
};

struct oldDendriteInfo {
  uint8 cversion;

  // rules have size 8 for C1, size 12 for C2

  uint8 srclobe;
  uint8 min;
  uint8 max;
  uint8 spread;
  uint8 fanout;
  uint8 minLTW;
  uint8 maxLTW;
  uint8 minstr;
  uint8 maxstr;
  uint8 migrateflag;
  uint8 relaxsuscept;
  uint8 relaxSTW;
  uint8 LTWgainrate;
  uint8 strgain;
  uint8 strloss;

  uint8 strgainrule[12];
  uint8 strlossrule[12];
  uint8 susceptrule[12];
  uint8 relaxrule[12];

  // back/forward propogation is C2 only
  uint8 backproprule[12];
  uint8 forproprule[12];

  friend ostream &operator << (ostream &, const oldDendriteInfo &);
  friend istream &operator >> (istream &, oldDendriteInfo &);

  oldDendriteInfo(uint8 v) { cversion = v; }
};

//! gene: old-style (c1/c2) brain lobe
class oldBrainLobeGene : public brainGene {
protected:
  uint8 subtype() const { return 0; }

  void write(ostream &) const;
  void read(istream &);

public:  
  // rules have size 8 for C1, size 12 for C2

  uint8 x;
  uint8 y;
  uint8 width;
  uint8 height;
  uint8 perceptflag;
  uint8 nominalthreshold;
  uint8 leakagerate;
  uint8 reststate;
  uint8 inputgain;
  uint8 flags;

  uint8 staterule[12];

  oldDendriteInfo dendrite1;
  oldDendriteInfo dendrite2;

  oldBrainLobeGene(uint8 v) : brainGene(v), dendrite1(v), dendrite2(v) { }
  virtual const char *name() { return "Lobe"; }
};

//! gene: c2e brain tract
class c2eBrainTractGene : public brainGene {
protected:
  uint8 subtype() const { return 2; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint16 updatetime;
  uint8 srclobe[4];
  uint16 srclobe_lowerbound;
  uint16 srclobe_upperbound;
  uint16 src_noconnections;
  uint8 destlobe[4];
  uint16 destlobe_lowerbound;
  uint16 destlobe_upperbound;
  uint16 dest_noconnections;
  uint8 migrates; // flag
  uint8 norandomconnections; // flag
  uint8 srcvar;
  uint8 destvar;
  uint8 initrulealways; // flag
  uint8 spare[5];
  uint8 initialiserule[48];
  uint8 updaterule[48];

  c2eBrainTractGene(uint8 v) : brainGene(v) { }
  const char *name() { return "Tract"; }
};

class bioGene : public gene {
protected:
  uint8 type() const { return 1; }

  bioGene(uint8 v) : gene(v) { }
  const char *typeName() { return "Biochemistry"; }
};

//! gene: receptor
class bioReceptorGene : public bioGene {
protected:
  uint8 subtype() const { return 0; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 organ;
  uint8 tissue;
  uint8 locus;
  uint8 chemical;
  uint8 threshold;
  uint8 nominal;
  uint8 gain;
  bool inverted; // 1 in flags
  bool digital; // 2 in flags

  bioReceptorGene(uint8 v) : bioGene(v) { }
  const char *name() { return "Receptor"; }
};

//! gene: emitter
class bioEmitterGene : public bioGene {
protected:
  uint8 subtype() const { return 1; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 organ;
  uint8 tissue;
  uint8 locus;
  uint8 chemical;
  uint8 threshold;
  uint8 rate;
  uint8 gain;
  bool clear; // 1 in flags
  bool digital; // 2 in flags
  bool invert; // 4 in flags

  bioEmitterGene(uint8 v) : bioGene(v) { }
  const char *name() { return "Emitter"; }
};

//! gene: reaction
class bioReactionGene : public bioGene {
protected:
  uint8 subtype() const { return 2; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 reactant[4];
  uint8 quantity[4];
  uint8 rate;

  bioReactionGene(uint8 v) : bioGene(v) { }
  const char *name() { return "Reaction"; }
};

//! gene: half-lives
class bioHalfLivesGene : public bioGene {
protected:
  uint8 subtype() const { return 3; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 halflives[256];

  bioHalfLivesGene(uint8 v) : bioGene(v) { }
  const char *name() { return "Half-Life"; }
};

//! gene: initial concentration
class bioInitialConcentrationGene : public bioGene {
protected:
  uint8 subtype() const { return 4; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 chemical;
  uint8 quantity;

  bioInitialConcentrationGene(uint8 v) : bioGene(v) { }
  const char *name() { return "Initial Concentration"; }
};

//! gene: neuroemitter
class bioNeuroEmitterGene : public bioGene {
protected:
  uint8 subtype() const { return 5; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 lobes[3];
  uint8 neurons[3];
  uint8 rate;
  uint8 chemical[4];
  uint8 quantity[4];

  bioNeuroEmitterGene(uint8 v) : bioGene(v) { }
  const char *name() { return "Neuro Emitter"; }
};

//! gene: creature-type base class
class creatureGene : public gene {
protected:
  uint8 type() const { return 2; }

  creatureGene(uint8 v) : gene(v) { }
  const char *typeName() { return "Creature"; }
};

//! gene: stimulus
class creatureStimulusGene : public creatureGene {
protected:
  uint8 subtype() const { return 0; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 stim;
  uint8 significance;
  uint8 sensoryneuron;
  uint8 intensity;
  bool modulate; // 1 in flags
  bool addoffset; // 2 in flags
  bool whenasleep; // 4 in flags
  bool silent[4]; // 16, 32, 64, 128 in flags
  uint8 drives[4];
  uint8 amounts[4];

  creatureStimulusGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Stimulus"; }
};

//! gene: genus (must be first gene in file, must only be one instance)
class creatureGenusGene : public creatureGene {
protected:
  uint8 subtype() const { return 1; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 genus;
  std::string mum; // length is: 4 for C1/C2, 32 for c2e
  std::string dad;

  creatureGenusGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Genus"; }
};

//! gene: appearance
class creatureAppearanceGene : public creatureGene {
protected:
  uint8 subtype() const { return 2; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 part;
  uint8 variant;
  uint8 species; // genusofdonor; not present in c1

  creatureAppearanceGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Appearance"; }
};

//! gene: pose
class creaturePoseGene : public creatureGene {
protected:
  uint8 subtype() const { return 3; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 poseno;
  uint8 pose[16];

  int poseLength() const { return (cversion == 3) ? 16 : 15; }
  std::string getPoseString() const { return std::string((char *)pose, poseLength()); }
  
  creaturePoseGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Pose"; }
};

//! gene: gait
class creatureGaitGene : public creatureGene {
protected:
  uint8 subtype() const { return 4; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 drive;
  uint8 pose[8];

  int gaitLength() const { return 8; }

  creatureGaitGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Gait"; }
};

//! gene: instinct
class creatureInstinctGene : public creatureGene {
protected:
  uint8 subtype() const { return 5; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 lobes[3];
  uint8 neurons[3];
  uint8 action;
  uint8 drive;
  uint8 level;

  creatureInstinctGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Instinct"; }
};

//! gene: pigment
class creaturePigmentGene : public creatureGene {
protected:
  uint8 subtype() const { return 6; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 color;
  uint8 amount;

  creaturePigmentGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Pigment"; }
};

//! gene: pigment bleed
class creaturePigmentBleedGene : public creatureGene {
protected:
  uint8 subtype() const { return 7; }

  void write(ostream &) const;
  void read(istream &);

public:
  uint8 rotation;
  uint8 swap;

  creaturePigmentBleedGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Pigment Bleed"; }
};

//! gene: facial expression
class creatureFacialExpressionGene : public creatureGene {
protected:
  uint8 subtype() const { return 8; }
  
  void write(ostream &) const;
  void read(istream &);

public:
  uint16 expressionno;
  uint8 weight;
  uint8 drives[4];
  uint8 amounts[4];

  creatureFacialExpressionGene(uint8 v) : creatureGene(v) { }
  const char *name() { return "Facial Expression"; }
};

#endif

/* vim: set noet: */
