# NeoRadarVSID
vSID adaptation for NeoRadar ATC Client

# Installation
- Download the latest release from the [Releases] according to your platform (Windows, Linux, MacOS).
- Extract the contents of the zip file to your NeoRadar plugins directory (./Documents/NeoRadar).
- add the following line to your NeoRadar list.yaml config file (./Documents/NeoRadar/package/'yourpackage'/system/list.yaml):
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
- delete theses line to avoid conflict with the NeoRadarVSID plugin:
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
At most, vSID will assign SID & CFL every 5 seconds
- Informations first appear white when unconfirmed (ie: not printed in the FP/TAG).<br>
- Right clicking on SID and CFL open their respective menu to confirm them.<br>
They will turn green when confirmed.<br>
- If another value is assigned to the SID or CFL, they will turn orange while displaying the new assigned value to indicate deviation from config.<br>
