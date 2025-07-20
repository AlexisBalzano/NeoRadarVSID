# NeoRadarVSID
vSID adaptation for NeoRadar ATC Client


# TODO
- Add TAG items
- Add TAG Function
- Add Color logic
- Add TAG logic
- add fecthing of informations (SID config file)



# Functionnalities
- Automatically assign best SID for the current flight plan and aircraft type. (for the moment use NeoRadar suggestedSID)
- Automatically set CFL based on assigned SID:
	- check [oaci].json for the CFL
- Color TAG item accordingly:
	- suggested SID & CFL : white
	- set SID & CFL: green
	- custom set SID & CFL: orange
	- error SID & CFL: red
- Add a rating requirement for plugin usage
