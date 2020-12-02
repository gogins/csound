/*
 * C S O U N D
 *
 * L I C E N S E
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef CHORD_SPACE_H
#define CHORD_SPACE_H
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
// Header file only library.
#include "Platform.hpp"
#ifdef SWIG
%module CsoundAC
%{
#include <algorithm>
#include <cfloat>
#include <ChordSpaceBase.hpp>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <eigen3/Eigen/Dense>
#include <Event.hpp>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <Score.hpp>
#include <set>
#include <sstream>
#include <vector>
%}
%include "std_string.i"
%include "std_vector.i"
#else
#include <algorithm>
// Header file only library.
#include <boost/algorithm/string.hpp>
// Header file only library.
#include <boost/math/special_functions/ulp.hpp>
#include <cfloat>
// Header file only library.
#include <ChordSpaceBase.hpp>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstdarg>
#include <eigen3/Eigen/Dense>
#include "Event.hpp"
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <random>
#include "Score.hpp"
#include <set>
#include <sstream>
#include <vector>
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"

namespace csound {
/** \file ChordSpace.hpp
This library implements a geometric approach to some common operations on 
chords in neo-Riemannian music theory for use in score generating procedures:

 -  Identifying whether a chord belongs to some equivalence class of music
    theory, or sending a chord to its equivalent within a representative
    ("normal") fundamental domain of some equivalence relation. The
    equivalence relations are octave (O), permutational (P), transpositional,
    (T), inversional (I), and their compounds OP, OPT (set-class or chord
    type), and OPTI (similar to prime form), among others.

 -  Causing chord progressions to move strictly within an orbifold that
    reoresents some equivalence class.

 -  Implementing chord progressions based on the L, P, R, D, K, and Q
    operations of neo-Riemannian theory (thus implementing some aspects of
    "harmony").

 -  Implementing chord progressions performed within a more abstract
    equivalence class by means of the closest voice-leading within a less
    abstract equivalence class (thus implementing some fundamentals of
    "counterpoint").
    
 -  Implementing "functional" or "Roman numeral" operations performed 
    using scales and scale degrees (thus implementing many fundamentals of 
    "pragmatic music theory").
    
# Definitions

Pitch is the perception of a distinct sound frequency. It is a logarithmic
perception; octaves, which sound 'equivalent' in some sense, represent
doublings or halvings of frequency.

Pitches and intervals are represented as real numbers. Middle C is 60 and the
octave is 12. Our usual system of 12-tone equal temperament, as well as MIDI
key numbers, are completely represented by the whole numbers; any and all
other pitches can be represented simply by using fractions.

A voice is a distinct sound that is heard as having a pitch.

A chord is simply a set of voices heard at the same time, represented here
as a point in a chord space having one dimension of pitch for each voice
in the chord.

A scale is a chord with a tonic pitch-class as its first and lowest voice, 
all other voices being pitch-classes sorted in ascending order.

For the purposes of algorithmic composition, a score can be considered to be a 
sequence of more or less fleeting chords.

# Equivalence Relations and Classes

An equivalence relation identifies different elements of a set as belonging to
the same class. For example the octave is an equivalence relation that 
identifies C1, C2, and C3 as belonging to the equivalence class C. Operations 
that send elements to their equivalents induce quotient spaces or orbifolds, 
where the equivalence operation identifies points on one facet of the orbifold 
with points on an opposing facet. The fundamental domain of the equivalence 
relation is the space consisting of the orbifold and its surface.

Plain chord space has no equivalence relation. Ordered chords are represented
as vectors in parentheses (p1, ..., pN). Unordered chords are represented as
sorted vectors in braces {p1, ..., pN}. Unordering is itself an equivalence
relation -- permutational equivalence.

The following equivalence relations apply to pitches and chords, and exist in
different orbifolds. Equivalence relations can be combined (Callendar, Quinn,
and Tymoczko, "Generalized Voice-Leading Spaces," _Science_ 320, 2008), and
the more equivalence relations are combined, the more abstract is the
resulting orbifold compared to the parent space.

In most cases, a chord space can be divided into a number, possibly
infinite, of geometrically equivalent fundamental domains for the same
equivalence relation. Therefore, here we use the notion of 'representative'
or 'normal' fundamental domain. For example, the representative fundamental
domain of unordered sequences, out of all possible orderings, consists of all
sequences in their ordinary sorted order. It is important, in the following,
to identify representative fundamental domains that combine properly, e.g.
such that the representative fundamental domain of OP / the representative
fundamental domain of PI equals the representative fundamental domain of OPI.
And this in turn may require accounting for duplicate elements of the
representative fundamental domain caused by reflections or singularities in
the orbifold (e.g. on vertices, edges, or facets shared by fundamental domains 
with a cyclical structure), or by doubled pitches in a chord.

<dl>
<dt>C   <dd>Cardinality equivalence, e.g. {1, 1, 2} == {1, 2}. _Not_ assuming
        cardinality equivalence ensures that there is a proto-metric in plain
        chord space that is inherited by all child chord spaces. Cardinality
        equivalence is never assumed here, because we are working in chord
        spaces of fixed dimensionality; e.g. we represent the note middle C
        not only as {60}, but also as {60, 60, ..., 60}.
        
<dt>O   <dd>Octave equivalence. The fundamental domain is defined by the pitches
        in a chord spanning the range of an octave or less, and summing to
        an octave or less.

<dt>P   <dd>Permutational equivalence. The fundamental domain is defined by a
        "wedge" of plain chord space in which the voices of a chord are always
        sorted by pitch.

<dt>T   <dd>Transpositional equivalence, e.g. {1, 2} == {7, 8}. The fundamental
        domain is defined as a hyperplane in chord space at right angles to the
        diagonal of unison chords. Represented by the chord always having a
        sum of pitches equal to 0.

<dt>Tg  <dd>Transpositional equivalence; the pitches of the chord are sent to 
        the ceilings of the pitches in the first chord whose sum is equal 
        to or greater than 0, i.e., rounded up to equal temperament.

<dt>I   <dd>Inversional equivalence. Care is needed to distinguish the
        mathematician's sense of 'invert', which means 'pitch-space inversion'
        or 'reflect in a point', from the musician's sense of 'invert', which
        varies according to context but in practice often means 'registral
        inversion' or 'revoice by adding an octave to the lowest tone of a
        chord.' Here, we use 'invert' and 'inversion' in the mathematician's
        sense, and we use the terms 'revoice' and 'voicing' for the musician's
        'invert' and 'inversion'. Here, the inversion of a chord is its 
        reflection in a hyperplane (the inversion flat) that divides a 
        fundamental domain of pitch.

<dt>PI  <dd>Inversional equivalence with permutational equivalence. The
        'inversion flat' of unordered chord space is a hyperplane consisting
        of all those unordered chords that are invariant under inversion. A
        fundamental domain is defined by any half space bounded by a
        hyperplane containing the inversion flat.

<dt>OP  <dd>Octave equivalence with permutational equivalence. Tymoczko's 
        orbifold for chords; i.e. chords with a fixed number of voices in a 
        harmonic context. The fundamental domain is defined as a hyperprism 
        one octave long with as many sides as voices and the ends identified 
        by octave equivalence and one cyclical permutation of voices, modulo 
        the unordering. In OP for trichords in 12TET, the augmented triads run 
        up the middle of the prism, the major and minor triads are in 6
        alternating columns around the augmented triads, the two-pitch chords
        form the 3 sides, and the one-pitch chords form the 3 edges that join
        the sides.
        
<dt>OPT  <dd>The layer of the OP prism as close as possible to the origin, modulo
        the number of voices. Chord type. Note that CM and Cm are different
        OPT. Because the OP prism is canted down from the origin, at least one
        pitch in each OPT chord (excepting the origin itself) is negative. 
        For n dimensions there are n OPT fundamental domains centering on the 
        maximally even chord and generated by rotation about the maximally 
        even chord, equivalently octavewise revoicing, more or less the same 
        as the musician's sense of "chord inversion."

<dt>OPTT  <dd>The same as OPT, but with chords rounded up within equal 
        temperament; equivalent to "chord type."

<dt>OPI  <dd>The OP prism modulo inversion, i.e. 1/2 of the OP prism. The
        representative fundamental consits of those chords having inversional 
        equivalence.

<dt>OPTI  <dd>The OPT layer modulo inversion, i.e. 1/2 of the OPT layer.
        Set-class. Note that minor and major triads are are the same OPTI.

<dt>OPTTI  <dd>The same as OPTI, but with chords rounded up within equal 
        temperament; equivalent to "set class."
</dl>

# Operations

Each of the above equivalence relations is, of course, an operation that sends
chords outside some fundamental domain to chords inside that fundamental 
domain. We define the following additional operations:

<dl>
<dt>T(p, x)     <dd>Translate p by x.

<dt>I(p [, x])  <dd>Reflect p in x, by default the origin.

<dt>P           <dd>Send a major triad to the minor triad with the same root,
                or vice versa (Riemann's parallel transformation).

<dt>L           <dd>Send a major triad to the minor triad one major third higher,
                or vice versa (Riemann's Leittonwechsel or leading-tone
                exchange transformation).

<dt>R           <dd>Send a major triad to the minor triad one minor third lower,
                or vice versa (Riemann's relative transformation).

<dt>D            <dd>Send a triad to the next triad a perfect fifth lower
                (dominant transformation).
</dl>

P, L, and R have been extended as follows, see Fiore and Satyendra,
"Generalized Contextual Groups", _Music Theory Online_ 11, August 2008:

<dl>
<dt>K(c)        <dd>Interchange by inversion;
                `K(c) := I(c, c[1] + c[2])`.
                This is a generalized form of P; for major and minor triads,
                it is exactly the same as P, but it also works with other
                chord types.

<dt>Q(c, n, m)  <dd>Contexual transposition;
                `Q(c, n, m) := T(c, n)` if c is a T-form of m,
                or `T(c, -n)` if c is an I-form of M. Not a generalized form
                of L or R; but, like them, K and Q generate the T-I group.
</dl>
                
*/

class SILENCE_PUBLIC ChordScore;

SILENCE_PUBLIC void apply(Score &score, const Chord &chord, double startTime, double endTime, bool octaveEquivalence = true);

/**
 * Score equipped with chords. The notes in the score may be conformed to the
 * chord that obtains at the time of the notes. The times and durations of
 * notes and chords are rescaled together. This is done by finding minimum and
 * maximum times by counting both note times and chord times.
 */
class SILENCE_PUBLIC ChordScore : public Score {
public:
    std::map<double, Chord> chords_for_times;
    /**
     * Conforms the pitch-classes of the events in this to the closest
     * pitch-class of the chord, if any, that obtains at that time.
     */
    virtual void conformToChords(bool tie_overlaps = true, bool octave_equivalence = true);
    /**
     * Returns a pointer to the first chord that starts at or after the
     * specified time. If there is no such chord, a null pointer is returned.
     */
    virtual Chord *getChord(double time_);
    double getDuration();
    void getScale(std::vector<Event> &score, int dimension, size_t beginAt, size_t endAt, double &minimum, double &range);
    virtual void insertChord(double tyme, const Chord chord);
    void setDuration(double targetDuration);
    void setScale(std::vector<Event> &score,
                  int dimension,
                  bool rescaleMinimum,
                  bool rescaleRange,
                  size_t beginAt,
                  size_t endAt,
                  double targetMinimum,
                  double targetRange);
};

/**
 * If the Event is a note, moves its pitch to the closest pitch of the chord.
 * If octaveEquivalence is true (the default), the pitch-class of the note is 
 * moved to the closest pitch-class of the chord, i.e. keeping the note more 
 * or less in its original register; otherwise, the pitch of the note is moved 
 * to the closest absolute pitch of the chord.
 */
SILENCE_PUBLIC void conformToChord(Event &event, const Chord &chord, bool octaveEquivalence = true);

/**
 * Returns a chord containing all the pitches of the score beginning at or 
 * later than the start time, and up to but not including the end time.
 */
SILENCE_PUBLIC Chord gather(Score &score, double startTime, double endTime);

/**
 * Inserts the notes of the chord into the score at the specified time.
 */
SILENCE_PUBLIC void insert(Score &score,
                                  const Chord &chord,
                                  double time_);
                                  
/**
 * Creates a complete "note on" Event for the indicated voice of the 
 * chord. If the optional duration, channel, velocity, and pan parameters
 * are not passed, then the Chord's own values for these are used.
 */
SILENCE_PUBLIC Event note(const Chord &chord, 
    int voice,
    double time_,
    double duration_ = DBL_MAX,
    double channel_ = DBL_MAX,
    double velocity_ = DBL_MAX,
    double pan_ = DBL_MAX);

/**
 * Returns an individual note for each voice of the chord. If the optional
 * duration, channel, velocity, and pan parameters are not passed, then 
 * the Chord's own values for these are used.
 */
SILENCE_PUBLIC Score notes(const Chord &chord,
    double time_,
    double duration_ = DBL_MAX,
    double channel_ = DBL_MAX,
    double velocity_ = DBL_MAX,
    double pan_ = DBL_MAX);

/**
 * Returns a slice of the Score starting at the start time and extending up
 * to but not including the end time. The slice contains pointers to the Events
 * in the Score.
 */
SILENCE_PUBLIC std::vector<Event *> slice(Score &score, double startTime, double endTime);

inline SILENCE_PUBLIC Chord gather(Score &score, double startTime, double endTime) {
    std::vector<Event *> slice_ = slice(score, startTime, endTime);
    std::set<double> pitches;
    for (int i = 0; i < slice_.size(); ++i) {
        pitches.insert(slice_[i]->getKey());
    }
    Chord chord;
    chord.resize(pitches.size());
    int voice = 0;
    for (std::set<double>::iterator it = pitches.begin(); it != pitches.end(); ++it) {
        chord.setPitch(voice, *it);
        voice++;
    }
    return chord;
}

inline Event note(const Chord &chord,
    int voice,
    double time_,
    double duration_,
    double channel_,
    double velocity_,
    double pan_) {
    Event note;
    note.setTime(time_);
    note.setKey(chord.getPitch(voice));
    if (duration_ != DBL_MAX) {
        note.setDuration(duration_);
    } else {
        note.setDuration(chord.getDuration(voice));
    }
    if (channel_ != DBL_MAX) {
        note.setInstrument(channel_);
    } else {
        note.setInstrument(chord.getInstrument(voice));
    }
    if (velocity_ != DBL_MAX) {
        note.setVelocity(velocity_);
    } else {
        note.setVelocity(chord.getLoudness(voice));
    }
    if (pan_ != DBL_MAX) {
        note.setPan(pan_);
    } else {
        note.setPan(chord.getPan(voice));
    }
    return note;
}

inline Score notes(const Chord &chord,
    double time_,
    double duration_,
    double channel_,
    double velocity_,
    double pan_) {
    Score score;
    for (int voice = 0; voice < chord.voices(); ++voice) {
        Event event = note(chord, voice, time_, duration_, channel_, velocity_, pan_);
        score.append(event);
    }
    return score;
}

inline SILENCE_PUBLIC std::vector<Event *> slice(Score &score, double startTime, double endTime) {
    std::vector<Event *> result;
    for (int i = 0, n = score.size(); i < n; ++i) {
        Event *event = &score[i];
        if (event->isNoteOn()) {
            double eventStart = event->getTime();
            if (eventStart >= startTime && eventStart < endTime) {
                result.push_back(event);
            }
        }
    }
    return result;
}

inline SILENCE_PUBLIC void toScore(const Chord &chord, 
    Score &score,
    double time_, bool voiceIsInstrument) {
    for (int voice = 0; voice < chord.voices(); ++voice) {
        double instrument = double(voice);
        if (!voiceIsInstrument) {
            instrument = chord.getInstrument(voice);
        }
        score.append(time_,
                     chord.getDuration(voice),
                     144.0,
                     instrument,
                     chord.getPitch(voice),
                     chord.getLoudness(voice),
                     0.0,
                     chord.getPan(voice));
    }
}

inline SILENCE_PUBLIC void apply(Score &score, const Chord &chord, double startTime, double endTime, bool octaveEquivalence) {
    std::vector<Event *> slice_ = slice(score, startTime, endTime);
    for (int i = 0; i < slice_.size(); ++i) {
        Event &event = *slice_[i];
        conformToChord(event, chord, octaveEquivalence);
    }
}

inline SILENCE_PUBLIC void conformToChord(Event &event, const Chord &chord, bool octaveEquivalence) {
    if (!event.isNoteOn()) {
        return;
    }
    double pitch = event.getKey();
    if (octaveEquivalence) {
        Chord pcs = chord.epcs();
        pitch = conformToPitchClassSet(pitch, pcs);
    } else {
        pitch = closestPitch(pitch, chord);
    }
    event.setKey(pitch);
}

inline SILENCE_PUBLIC void ChordScore::insertChord(double tyme, const Chord chord) {
    chords_for_times[tyme] = chord;
}
/**
 * Returns a pointer to the first chord that starts at or after the
 * specified time. If there is no such chord, a null pointer is returned.
 */
inline SILENCE_PUBLIC Chord *ChordScore::getChord(double time_) {
    auto it = chords_for_times.lower_bound(time_);
    if (it != chords_for_times.end()) {
        return &it->second;
    } else {
        return nullptr;
    }
}

/**
 * Conforms the pitch-classes of the events in this to the closest
 * pitch-class of the chord, if any, that obtains at that time.
 */
inline SILENCE_PUBLIC void ChordScore::conformToChords(bool tie_overlaps, bool octave_equivalence) {
    sort();
    if (chords_for_times.begin() != chords_for_times.end()) {
        for (auto event_iterator = begin(); event_iterator != end(); ++event_iterator) {
            auto chord_iterator = chords_for_times.lower_bound(event_iterator->getTime());
            if (chord_iterator != chords_for_times.end()) {
                conformToChord(*event_iterator, chord_iterator->second, octave_equivalence);
            }
        }
    }
    if (tie_overlaps == true) {
        tieOverlappingNotes(true);
    }
}

inline SILENCE_PUBLIC void ChordScore::getScale(std::vector<Event> &score, int dimension, size_t beginAt, size_t endAt, double &minimum, double &range)
{
    if(beginAt == endAt) {
        return;
    }
    const Event &beginEvent = score[beginAt];
    double maximum = beginEvent[dimension];
    const Event &endEvent = score[endAt - 1];
    minimum = endEvent[dimension];
    if(dimension == Event::TIME) {
        const Event &e = score[beginAt];
        maximum = std::max(e.getTime(), e.getTime() + e.getDuration());
        minimum = std::min(e.getTime(), e.getTime() + e.getDuration());
        double beginning;
        double ending;
        for( ; beginAt != endAt; ++beginAt) {
            const Event &event = score[beginAt];
            beginning = std::min(event.getTime(), event.getTime() + event.getDuration());
            ending = std::max(event.getTime(), event.getTime() + event.getDuration());
            if(ending > maximum) {
                maximum = ending;
            } else if(beginning < minimum) {
                minimum = beginning;
            }
        }
        // Also take into account chord times.
        auto chord_begin = chords_for_times.begin();
        auto chord_rbegin = chords_for_times.rbegin();
        if (chord_begin != chords_for_times.end() && chord_rbegin != chords_for_times.rend()) {
            minimum = std::min(minimum, chord_begin->first);
            maximum = std::max(maximum, chord_rbegin->first);
        }
    } else {
        for( ; beginAt != endAt; ++beginAt) {
            const Event &event = score[beginAt];
            if(event[dimension] > maximum) {
                maximum = event[dimension];
            }
            if(event[dimension] < minimum) {
                minimum = event[dimension];
            }
        }
    }
    range = maximum - minimum;
}

inline SILENCE_PUBLIC void ChordScore::setScale(std::vector<Event> &score,
              int dimension,
              bool rescaleMinimum,
              bool rescaleRange,
              size_t beginAt,
              size_t endAt,
              double targetMinimum,
              double targetRange)
{
    if(!(rescaleMinimum || rescaleRange)) {
        return;
    }
    if(beginAt == endAt) {
        return;
    }
    double actualMinimum;
    double actualRange;
    getScale(score, dimension, beginAt, endAt, actualMinimum, actualRange);
    double scale;
    if(actualRange == 0.0) {
        scale = 1.0;
    } else {
        scale = targetRange / actualRange;
    }
    for( ; beginAt != endAt; ++beginAt) {
        Event &event = score[beginAt];
        event[dimension] = event[dimension] - actualMinimum;
        if(rescaleRange) {
            event[dimension] = event[dimension] * scale;
        }
        if(rescaleMinimum) {
            event[dimension] = event[dimension] + targetMinimum;
        } else {
            event[dimension] = event[dimension] + actualMinimum;
        }
    }
    // Also rescale chord times.
    if (dimension == Event::TIME) {
        std::map<double, Chord> temp;
        for (auto it = chords_for_times.begin(); it != chords_for_times.end(); ++it) {
            double tyme = it->first;
            const Chord &chord = it->second;
            tyme = tyme - actualMinimum;
            if (rescaleRange) {
                tyme = tyme * scale;
            }
            if (rescaleMinimum) {
                tyme = tyme + targetMinimum;
            } else {
                tyme = tyme + actualMinimum;
            }
            temp[tyme] = chord;
        }
        chords_for_times = temp;
    }
}

inline SILENCE_PUBLIC double ChordScore::getDuration()
{
    double start = 0.0;
    double end = 0.0;
    for (int i = 0, n = size(); i < n; ++i) {
        const Event &event = at(i);
        if (i == 0) {
            start = event.getTime();
            end = event.getOffTime();
        } else {
            if (event.getTime() < start) {
                start = event.getTime();
            }
            if (event.getOffTime() > end) {
                end = event.getOffTime();
            }
        }
    }
    auto chord_begin = chords_for_times.begin();
    auto chord_rbegin = chords_for_times.rbegin();
    if (chord_begin != chords_for_times.end() && chord_rbegin != chords_for_times.rend()) {
        start = std::min(start, chord_begin->first);
        end = std::max(end, chord_rbegin->first);
    }
    return end - start;
}

inline SILENCE_PUBLIC void ChordScore::setDuration(double targetDuration)
{
    double currentDuration = getDuration();
    if (currentDuration == 0.0) {
        return;
    }
    double factor = targetDuration / currentDuration;
    for (size_t i = 0, n = size(); i < n; i++) {
        Event &event = (*this)[i];
        double time_ = event.getTime();
        double duration = event.getDuration();
        event.setTime(time_ * factor);
        event.setDuration(duration * factor);
    }
    std::map<double, Chord> temp;
    for (auto it = chords_for_times.begin(); it != chords_for_times.end(); ++it) {
        double tyme = it->first;
        const Chord &chord = it->second;
        tyme = tyme * factor;
        temp[tyme] = chord;
    }
    chords_for_times = temp;
}

inline SILENCE_PUBLIC void insert(Score &score,
                                  const Chord &chord,
                                  double time_, bool voice_is_instrument) {
    toScore(chord, score, time_, voice_is_instrument);
}


} // End of namespace csound.

#pragma GCC diagnostic push

#endif
  