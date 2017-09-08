<?php

namespace Drupal\audiofield\Plugin\AudioPlayer;

use Drupal\Core\Field\FieldItemListInterface;
use Drupal\audiofield\AudioFieldPluginBase;

/**
 * Implements the Wavesurfer Audio Player plugin.
 *
 * @AudioPlayer (
 *   id = "wavesurfer_audio_player",
 *   title = @Translation("Wavesurfer audio player"),
 *   description = @Translation("A customizable audio waveform visualization, built on top of Web Audio API and HTML5 Canvas."),
 *   fileTypes = {
 *     "mp3", "ogg", "oga", "wav",
 *   },
 *   libraryName = "wavesurfer",
 *   website = "https://github.com/katspaugh/wavesurfer.js",
 * )
 */
class WavesurferAudioPlayer extends AudioFieldPluginBase {

  /**
   * {@inheritdoc}
   */
  public function renderPlayer(FieldItemListInterface $items, $langcode, array $settings) {
    // Check to make sure we're installed.
    if (!$this->checkInstalled()) {
      // Show the error.
      $this->showInstallError();

      // Simply return the default rendering so the files are still displayed.
      $default_player = new DefaultMp3Player();
      return $default_player->renderPlayer($items, $langcode, $settings);
    }

    // Create arrays to pass to the twig template.
    $template_files = [];

    // Start building settings to pass to the javascript wavesurfer builder.
    $player_settings = [
      // Projekktor expects this as a 0 - 1 range.
      'volume' => ($settings['audio_player_initial_volume'] / 10),
      'playertype' => ($settings['audio_player_wavesurfer_combine_files'] ? 'playlist' : 'default'),
      'files' => [],
      'autoplay' => $settings['audio_player_autoplay'],
    ];

    // Format files for output.
    $template_files = $this->getItemRenderList($items);
    foreach ($template_files as $renderInfo) {
      // Add this file to the render settings.
      $player_settings['files'][] = [
        'id' => 'wavesurfer_' . $renderInfo->id,
        'path' => $renderInfo->url->toString(),
      ];
    }

    return [
      'audioplayer' => [
        '#theme' => 'audioplayer',
        '#plugin_id' => 'wavesurfer',
        '#plugin_theme' => $player_settings['playertype'],
        '#files' => $template_files,
      ],
      'downloads' => $this->createDownloadList($items, $settings),
      '#attached' => [
        'library' => [
          // Attach the wavesurfer library.
          'audiofield/audiofield.' . $this->getPluginLibraryName(),
        ],
        'drupalSettings' => [
          'audiofieldwavesurfer' => [
            $renderInfo->id => $player_settings,
          ],
        ],
      ],
    ];
  }

}
