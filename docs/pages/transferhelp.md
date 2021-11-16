###### [Home](../../index.md)
## Transfer Window

<img src="../../docs/images/PluginWindowMain.png" width="400">

This window uses the Reaper render queue to automate importing audio into Wwise.
It uses Waapi to communicate with the Wwise authoring tool, so depends on having the Wwise project open and Waapi configured.

The basic workflow for using the plugin is;

<details>
  <summary>Workflow</summary>

  1. In Reaper, instead of directly rendering your audio files, add them to the render queue\
  <img src="../../docs/images/renderqueue.png" width="400">\
  2. The plugin will show the render job and the resulting files in the main window\
  <img src="../../docs/images/transferjobs.png" width="400">\
  3. The top section of the plugin is where you set and apply the import properties for the render queue jobs\
  <img src="../../docs/images/transferproperties.png" width="400">\
  4. Once you have set the properties, ensure you have the desired parent selected in Wwise, then select the desired job and hit Apply\
  <img src="../../docs/images/transferapplied.png" width="400">\
  5. Once all jobs have their import properties set, you are ready to Render and Import\
  <img src="../../docs/images/transferbuttons.png" width="400">\
  This will start Reaper rendering, and when the render is finished the plugin will import the resulting files into Wwise using the import settings\

</details>
