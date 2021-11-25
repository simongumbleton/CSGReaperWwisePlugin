###### [Home](../../index.md)
## Transfer Window

<img src="../../docs/images/PluginWindowMain.png" width="400">

This window uses the Reaper render queue to automate importing audio into Wwise.
It uses Waapi to communicate with the Wwise authoring tool, so depends on having the Wwise project open and Waapi configured.  

The title summarises the basic idea of this plugin window.  
<img src="../../docs/images/transfertitle.png" width="800"> <br />

### The basic workflow for using the plugin is;
<details>
  <summary> Workflow </summary>
<br />
  1. In Reaper, instead of directly rendering your audio files, add them to the render queue <br />
  <img src="../../docs/images/renderqueue.png" width="400"> <br />
  <br />
  2. The plugin will show the render job and the resulting files in the main window <br />
  <img src="../../docs/images/transferjobs.png" width="400"> <br />
  <br />
  3. The top section of the plugin is where you set and apply the import properties for the render queue jobs <br />
  <img src="../../docs/images/transferproperties.png" width="400"> <br />
  <br />
  4. Once you have set the properties, ensure you have the desired parent selected in Wwise, then select the desired job and hit Apply <br />
  <img src="../../docs/images/transferapplied.png" width="400"> <br />
  <br />
  5. Once all jobs have their import properties set, you are ready to Render and Import <br />
  <img src="../../docs/images/transferbuttons.png" width="400"> <br />
  This will start Reaper rendering, and when the render is finished the plugin will import the resulting files into Wwise using the import settings <br />

</details>

****

### Setting the import properties;
<details>
  <summary> Import Properties </summary>
  <br />
  <strong> - At the top of the window, the plugin will show the currently selected Wwise object. </strong> <br />
  This will update when the selection changes in Wwise. <br />
  <img src="../../docs/images/transferselectedparent.png" width="400"> <br />
  <br />
 - <strong>Choose to import the audio files as sound fx, voice or music tracks. When voice is selected a project language drop-down is enabled </strong> <br />
  <img src="../../docs/images/transferimportas.png" width="400"> <br />
  <br />
 - <strong>Choose to create events for the imported items </strong> <br />
    Play@Children will create a Play event for each audio file in the list when it is imported <br />
    Play@Parent will create a Play event for the parent object of the imported files. For example, if you are importing variations into a random container <br />
  <img src="../../docs/images/transferevents.png" width="400"> <br />
  <br />  
 - <strong>Choose the location in Originals to copy the audio files into </strong> <br />
  Originals dir matches Wwise - This will create a folder structure matching the Actor-Mixer structure where the audio is imported (can result is fairly deep nesting as it mirrors the AM structure fully) <br />
  Otherwise the user can either enter the text path, relative to the SFX/Voices root, or use the system directory selector <br />
  <img src="../../docs/images/transferoriginals.png" width="800"> <br />
  <br /> 
 -<strong> Refresh the list of render jobs and files in the plugin window </strong> <br />
  <img src="../../docs/images/transferrefreshjob.png" width="400"> <br />
  <br />
 -<strong> The status bar shows and errors, as well as the current Wwise connection. </strong> <br />
  There is also a button to refresh the connection to Wwise in cases where the plugin was open without Wwise open, or the Wwise tool was closed while the plugin was in use. <br />
  <img src="../../docs/images/transferconnectionstatus.png" width="400"> <br />
  <br />
  
</details>

****

### Render jobs and per file overrides;
<details>
  <summary> Render Job and File Overrides </summary>
  <br />
   -<strong> The central view of the plugin shows the list of jobs in the render queue, and the files that will be created from them </strong> <br />
  The list is automatically populated when the plugin is opened <br />
  <img src="../../docs/images/transferjobs.png" width="400"> <br />
  <br />
  -<strong> If the render queue changes, you can refresh the list of jobs and files with the Refresh button </strong> <br />
  Refreshing the list will clear any applied properties <br />
  <img src="../../docs/images/transferrefreshjob.png" width="400"> <br />
  <br />
   -<strong> Applying properties to the job itself means all the files in that job will use the same properties </strong> <br />
  It is possible to override individual files in the job with different properties by selecting the file instead of the job when applying <br />
  <img src="../../docs/images/transferjoboverride.png" width="400"> <br />
  <br />
  -<strong> It is possible to have multiple jobs in the render queue, and the plugin will display multiple jobs in the list </strong> <br />
  This is useful for cases where you want to export batches of audio with different render settings, or render from multiple projects <br />
  <img src="../../docs/images/transferjobmulti.png" width="400"> <br />
  <br />
  
</details>

****

### Wwise Connection and Status;
<details>
  <summary> Wwise Connection and Status </summary>
  <br />
  -<strong> When the plugin launches, it tries to connect to a Wwise project via Waapi. You must ensure that Waapi is enabled in the Wwise authoring tool </strong> <br />
  Take note of the WAMP port you are using. 8080 is the default for the plugin, but it can be changed in the plugin config file <br />
  <img src="../../docs/images/wwisewaapisetup.png" width="400"> <br />
  <br />
  -<strong> If the plugin has a connection, it will display the status and name of the Wwise project at the bottom</strong> <br />
  If the connection is lost, the Connect To Wwise button allows you to try and re-establish a connection<br />
  <img src="../../docs/images/wwisestatusok.png" width="800"> <br />
  <br />
    -<strong> If the plugin cannot make a connection, it will display a no connection warning</strong> <br />
  The most likely cause of this is a mismatch in the ports being used by Wwise and the plugin<br />
  <img src="../../docs/images/statusnowwise.png" width="800"> <br />
  <br />
</details>

****

### Version Handling and Existing Files;
<details>
  <summary> Version Handling and Existing Files </summary>
    <br />
  -<strong> During import, the plugin will look for already existing sounds in Wwise with matching names, underneath the selected parent </strong> <br />
  If an already existing sound is found, the plugin will just update the wav file<br />
  <img src="../../docs/images/replaceexistingfile.png" width="400"> <br />
  <br />
  -<strong> The plugin can also support versioning, with the use of a pre-defined version token</strong> <br />
  This token is specified in the plugin config file, by default the token is; "_v00".<br />
  The plugin will try to match the token pattern in the audio filename to decide if the file to import is a version<br />
  <img src="../../docs/images/versiontoken.png" width="400"> <br />
  <br />
    -<strong> If the file to import is a Version, it is imported as a new source underneath the existing sound</strong> <br />
  The plugin also updates the work unit xml to set this new source as the Active Source (used) <br />
  <img src="../../docs/images/wwiseversioning.png" width="400"> <br />
  <br />
</details>

****