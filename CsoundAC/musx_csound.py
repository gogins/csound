'''
Author: Michael Gogins

This module translates MIDI files generated by musx to standard Csound scores
where each note is an "i" statement in the format:
```
i instrument_number time_seconds duration_seconds midi_key midi_velocity
```
Assigns a duration to the event at the given index in the given MIDI sequence.
If the event is a note on event, the duration is the difference between the 
time of the first matching note off message and the time of the note on event.
'''
from musx.midi import MidiNote, MidiSeq, MidiFile


def assign_duration(index, midiseq):
    event = midiseq[index]
    event.duration = 0
    if event.is_note_on() == True:
        for off_index in range(index, len(midiseq)):
            note_off = midiseq[off_index]
            if note_off.is_note_off():
                if note_off.keynum() == event.keynum() and note_off.channel() == event.channel():
                    event.duration = note_off.time - event.time
                    break
    return event
    
'''
Assigns a duration to each MIDI event in each MIDI sequence in the MIDI file.
'''
def assign_durations(midifile):
    for track in midifile:
        for index in range(len(track)):
            event = assign_duration(index, track)
            if event.is_note_on() == True:
                i_statement = to_i_statement(event)
'''
Translates the MIDI event into a Csound "i" statement.
'''
def to_i_statement(midi_event):
    i_statement = "i {} {} {} {} {}".format(1 + midi_event.channel(), midi_event.time, midi_event.duration, midi_event.keynum(), midi_event.velocity())
    return i_statement
    
'''
Returns a Csound score file as a single string, translated from the MIDI file.
It is assumed that times are in seconds.
'''
def to_csound_score(midifile):
    assign_durations(midifile)
    csound_score = []
    for track in midifile:
        for event in track:
            if event.is_note_on():
                if event.duration > 0:
                    i_statement = to_i_statement(event)
                    csound_score.append(i_statement + "\n")
    return ''.join(csound_score)
                
    
