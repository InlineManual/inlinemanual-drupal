<?php

namespace Drupal\inlinemanual;

use Drupal\Core\Extension\ModuleHandlerInterface;
use Drupal\Core\Form\ConfigFormBase;
use Drupal\Core\Config\ConfigFactory;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Configure inlinemanual settings for this site.
 */
class InlineManualSettingsForm extends ConfigFormBase {

  /**
   * The module handler.
   *
   * @var \Drupal\Core\Extension\ModuleHandlerInterface
   */
  protected $moduleHandler;

  /**
   * Constructs a \Drupal\user\StatisticsSettingsForm object.
   *
   * @param \Drupal\Core\Config\ConfigFactory $config_factory
   *   The factory for configuration objects.
   * @param \Drupal\Core\Extension\ModuleHandlerInterface $module_handler
   *   The module handler.
   */
  public function __construct(ConfigFactory $config_factory, ModuleHandlerInterface $module_handler) {
    parent::__construct($config_factory);

    $this->moduleHandler = $module_handler;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('config.factory'),
      $container->get('module_handler')
    );
  }

  /**
   * {@inheritdoc}
   */
	public function getFormId() {
    return 'inlinemanual_settings_form';
  }

  /**
   * Implements \Drupal\Core\Form\FormInterface::buildForm().
   */
  public function buildForm(array $form, array &$form_state) {
    $config = $this->configFactory->get('inlinemanual.settings');

    $form['inlinemanual_site_key'] = array(
      '#title' => t('Site API Key'),
      '#type' => 'textfield',
      '#default_value' => $config->get('site_key'),
      '#size' => 60,
      '#maxlength' => 120,
      '#required' => TRUE,
      '#description' => t('This ID is unique to each site and can be found on inline manual portal.'),
    );

    $form['inlinemanual_widget_title'] = array(
      '#title' => t('Widget title'),
      '#type' => 'textfield',
      '#default_value' => $config->get('widget.title'),
      '#size' => 60,
      '#maxlength' => 120,
      '#required' => FALSE,
      '#description' => t('The title of the widget that end-users see.'),
    );

    // add option to change the colour of the widget
    $form['#attached']['css'] = array('core/assets/vendor/farbtastic/farbtastic.css');
    $form['#attached']['js'] = array('core/assets/vendor/farbtastic/farbtastic.js');

    $form ['inlinemanual_widget_color'] = array(
     '#type' => 'textfield',
     '#title' => t('Widget Color'),
     '#default_value' => $config->get('widget.color', '#222222'),
     '#description' => '<div id="inlinemanual_widget_colorpicker"></div>',
     '#size' => 9,
     '#maxlength' => 7,
     '#suffix' => "<script type='text/javascript'>
       jQuery(document).ready(function() {
         jQuery('#inlinemanual_widget_colorpicker').farbtastic('#edit-inlinemanual-widget-color');
       });
     </script>"
    );
    $form ['inlinemanual_widget_text_color'] = array(
     '#type' => 'textfield',
     '#title' => t('Widget Text Color'),
     '#default_value' => $config->get('widget.text_color', '#ffffff'),
     '#description' => '<div id="inlinemanual_widget_text_colorpicker"></div>',
     '#size' => 9,
     '#maxlength' => 7,
     '#suffix' => "<script type='text/javascript'>
       jQuery(document).ready(function() {
         jQuery('#inlinemanual_widget_text_colorpicker').farbtastic('#edit-inlinemanual-widget-text-color');
       });
     </script>"
    );

    return parent::buildForm($form, $form_state);
  }

  /**
   * Implements \Drupal\Core\Form\FormInterface::submitForm().
   */
  public function submitForm(array &$form, array &$form_state) {
    $this->configFactory->get('inlinemanual.settings')
      ->set('site_key', $form_state['values']['inlinemanual_site_key'])
      ->set('widget.title', $form_state['values']['inlinemanual_widget_title'])
      ->set('widget.color', $form_state['values']['inlinemanual_widget_color'])
      ->set('widget.text_color', $form_state['values']['inlinemanual_widget_text_color'])
      ->save();

    // The popular statistics block is dependent on these settings, so clear the
    // block plugin definitions cache.
    if ($this->moduleHandler->moduleExists('block')) {
      \Drupal::service('plugin.manager.block')->clearCachedDefinitions();
    }

    parent::submitForm($form, $form_state);
  }  
}