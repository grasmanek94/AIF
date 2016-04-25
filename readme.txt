AIF - Auto Installer Feature (for SA-MP) plugin.

This is the alpha package of AIF 1.0.
It is fully compatible with Windows but needs a Linux port.

The goal of the plugin is to simplify installation of server packages to the end user.

.AIFPAK files are zip files and .AIFLIST files are text files.

HowTo:
Download the package.
Put AIF.dll into your "plugins" folder.
Add "AIF" to your plugins line in the server.cfg (open with wordpad).
NOTE: MAKE SURE 'AIF' IS THE FIRST PLUGIN THAT GETS LOADED (so it needs to be the closest to the 'plugins' text)
Else just make a 'plugins' line and repeat previous step.

Now you are ready to download packages and let them install themselves.
Download any .AIFPAK file and put it in your server's main folder (on windows this is where samp-server.exe is located).
Run the server and any packages will be automatically installed/updated. 
Now you can just download the required include and use it in your mode with no need to worry about bad/outdated plugins/scriptfiles installations!

For developers:
To see how to make automated installer packages open RouteConnector_173c.AIFPAK with any archive viewer that supports ZIP and look at INSTALL.AIFLIST.
If you want to see how to require (multiple) packages to be installed before allowing installation of your package see NFSUG2DPCv1.2.AIFPAK