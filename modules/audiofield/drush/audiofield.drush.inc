<?php

/**
 * @file
 * Drush integration for audiofield.
 */

/**
 * Implements hook_drush_command().
 *
 * Defining a drush command to install the Audiofield external libraries.
 */
function audiofield_drush_command() {
  $items = [];

  $items['audiofield-download'] = [
    'description' => dt('Downloads the suggested Audiofield libraries from their remote repos.'),
    'callback' => 'audiofield_drush_lib_download',
    'arguments' => [
      'library' => dt('The name of the library to install. If ommitted, all libraries will be installed.'),
    ],
  ];
  return $items;
}

/**
 * Implements hook_drush_help().
 *
 * Help description for the audiofield download command.
 */
function audiofield_drush_help($section) {
  switch ($section) {
    case 'drush:audiofield-download':
      return dt("Downloads the AudioField external libraries.");
  }
}

/**
 * Drush command callback.
 *
 * Installs the external libraries required for Audiofield plugins.
 *
 * @param string $installLibrary
 *   The name of the library to be installed, or blank for all libraries.
 */
function audiofield_drush_lib_download($installLibrary = '') {
  // Get a list of the audiofield plugins.
  $pluginList = \Drupal::service('plugin.manager.audiofield')->getDefinitions();

  // If there is an argument, check to make sure its valid.
  if (!empty($installLibrary)) {
    if (!isset($pluginList[$installLibrary . '_audio_player'])) {
      drush_log(dt('Error: @library is not a valid Audiofield library.', [
        '@library' => $installLibrary,
      ], 'error'), 'error');
      return;
    }
    // If the argument is valid, we only want to install that plugin.
    $pluginList = [$installLibrary . '_audio_player' => $pluginList[$installLibrary . '_audio_player']];
  }

  // Loop over each plugin and make sure it's library is installed.
  foreach ($pluginList as $pluginName => $plugin) {
    // Create an instance of this plugin.
    $pluginInstance = \Drupal::service('plugin.manager.audiofield')->createInstance($pluginName);

    // Only check install if there is a library for the plugin.
    if ($pluginInstance->getPluginLibrary()) {
      // Check if the plugin is installed and that there is an available source.
      if (!$pluginInstance->checkInstalled()) {
        // Get the library install path.
        $path = DRUPAL_ROOT . $pluginInstance->getPluginLibraryPath();
        // Create the install directory if it does not exist.
        if (!is_dir($path)) {
          drush_mkdir($path);
        }
        // Attempt to download and unzip.
        if (drush_op('chdir', $path) &&
          drush_shell_exec('wget ' . $pluginInstance->getPluginRemoteSource() . ' -O audiofield-dl.zip') &&
          drush_shell_exec('unzip audiofield-dl.zip') &&
          drush_shell_exec('rm -rf audiofield-dl.zip')
          ) {
          // If the library still is not installed, we need to move files.
          if (!$pluginInstance->checkInstalled()) {
            // Find all folders in this directory and move their
            // subdirectories up to the parent directory.
            drush_shell_exec('ls');
            foreach (drush_shell_exec_output() as $dir) {
              if (is_dir($dir)) {
                drush_shell_exec('mv ' . $dir . '/* .');
                drush_shell_exec('rm -rf ' . $dir);
              }
            }
          }
          if ($pluginInstance->checkInstalled()) {
            drush_log(dt('Audiofield library for @library has been successfully installed at @location', [
              '@library' => $pluginInstance->getPluginTitle(),
              '@location' => $pluginInstance->getPluginLibraryPath(),
            ], 'success'), 'success');
          }
          else {
            drush_log(dt('Error: unable to install Audiofield library @library', [
              '@library' => $pluginInstance->getPluginTitle(),
            ], 'error'), 'error');
          }
        }
        else {
          drush_log(dt('Error: unable to download Audiofield library @library', [
            '@library' => $pluginInstance->getPluginTitle(),
          ], 'error'), 'error');
        }
      }
      else {
        drush_log(dt('Audiofield library for @library is already installed at @location', [
          '@library' => $pluginInstance->getPluginTitle(),
          '@location' => $pluginInstance->getPluginLibraryPath(),
        ], 'success'), 'ok');
      }
    }
  }
}

/**
 * Implements drush_MODULE_post_COMMAND().
 *
 * This is used to install libraries after the module is enabled in drush.
 */
function drush_audiofield_post_pm_enable() {
  $extensions = func_get_args();
  // Deal with comma delimited extension list.
  if (strpos($extensions[0], ',') !== FALSE) {
    $extensions = explode(',', $extensions[0]);
  }

  if (in_array('audiofield', $extensions) && !drush_get_option('skip')) {
    if (drush_confirm('Do you want to install external libraries for use with Audiofield? (Note: you can install manually later)')) {
      audiofield_drush_lib_download();
    }
  }
}
