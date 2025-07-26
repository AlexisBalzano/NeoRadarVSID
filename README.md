# NeoRadarVSID
vSID adaptation for NeoRadar ATC Client <br>
<br>
<img width="565" height="154" alt="VSIDillustrationImage" src="https://github.com/user-attachments/assets/19cf296e-ca41-4fa3-a2b1-c3852d1df9a3" />

# Installation
- Download the latest release from the [Releases] according to your platform (Windows, Linux, MacOS).
- Extract the contents of the zip file to your NeoRadar plugins directory _(./Documents/NeoRadar)_.
- Either:
  - replace your `list.yaml` _(./Documents/NeoRadar/package/'yourpackage'/system/list.yaml)_ file with the one provided in the release
  - add the following line to your NeoRadar `list.yaml` config file :
```yaml
    - name: vrwy
      width: 30
      tagItem:
        itemName: plugin:NeoVSID:TAG_RWY
        leftClick: plugin:NeoVSID:ACTION_confirmRWY
        rightClick: sidMenu
    - name: vsid
      width: 80
      tagItem:
        itemName: plugin:NeoVSID:TAG_SID
        leftClick: plugin:NeoVSID:ACTION_confirmSID
        rightClick: sidMenu
    - name: vcfl
      width: 40
      tagItem:
        itemName: plugin:NeoVSID:TAG_CFL
        leftClick: plugin:NeoVSID:ACTION_confirmCFL
        rightClick: cfl
```
- delete theses line to avoid conflict with the NeoRadarVSID plugin (if not replacing with the provided file):
```yaml
    - name: rwy
      width: 30
      tagItem:
        itemName: text
        value:
          - =suggestedDepRunwayIfNoAssigned
          - |
            &depRunway
        color:
          - =suggestedDepRunwayColor
          - =blueDepRunway
        leftClick: sidMenu
        placeholder: "---"
    - name: sid
      width: 80
      tagItem:
        itemName: text
        value:
          - =suggestedSidIfNoAssigned
          - |
            &sid
        color:
          - =suggestedSidColor
          - =blueAssignedSid
        leftClick: sidMenu
        placeholder: "----"
    - name: cfl
      width: 40
      tagItem:
        itemName: clearedFlightLevel
        placeholder: "---"
        padWithZeroLeft: 5
        truncate: [0, 3]
        leftClick: cfl
```

# Usage
- Start NeoRadar and assign runways.<br>
At most, vSID will assign SID & CFL **every 5 seconds**.
- Informations first appear white when unconfirmed (ie: not printed in the FP/TAG).<br>
- Right clicking on SID and CFL open their respective menu to confirm them.<br>
They will turn green when confirmed.<br>
- If another value is assigned to the SID or CFL, they will turn orange while displaying the new assigned value to indicate deviation from config.<br>

# Commands
- `.vsid help` : display all available commands and their usage.<br>
- `.vsid version` : display the current version of the plugin.<br>
- `.vsid reset` : reset all plugin configurations.<br>
- `.vsid airports` : display all currently active airports <br>
- `.vsid auto` : **[COMMING SOON\]** toggle automatic RWY, SID & CFL assignment.<br>
- `.vsid pilots` : display all currently active pilots.<br>
- `.vsid rules` : display all currently loaded rules and their active state.<br>
- `.vsid areas` : display all currently loaded areas and their active state.<br>
- `.vsid rule <OACI> <RULENAME>` : toggle rule for the given OACI and rule name.<br>
- `.vsid area <OACI> <AREANAME>` : toggle area for the given OACI and area name.<br>
- `.vsid position <CALLSIGN> <AREANAME>` (*debug command*) : to check pilot position and if in area.<br>
- `.vsid remove <CALLSIGN>` (*debug command*) : remove pilot from the plugin (it will be readded on next plugin update if matching criteria, used to remove stuck aircraft).<br>
