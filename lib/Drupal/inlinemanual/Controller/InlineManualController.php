<?php

/**
 * @file
 * Contains \Drupal\aggregator\Controller\AggregatorController.
 */

namespace Drupal\inlinemanual\Controller;

use Drupal\Core\Controller\ControllerBase;

/**
 * Returns responses for aggregator module routes.
 */
class InlineManualController extends ControllerBase {

  /**
   * Page callback for "Refresh topics"
   */
  public function refresh() {
    inlinemanual_topics_fetch_all();
  
    drupal_set_message($this->t('Topics refreshed'));
    
    return $this->redirect('inlinemanual.topics');
  }
}
