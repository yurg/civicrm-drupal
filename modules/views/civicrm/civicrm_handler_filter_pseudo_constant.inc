<?php
/*
 +--------------------------------------------------------------------+
 | CiviCRM version 4.6                                                |
 +--------------------------------------------------------------------+
 | This file is a part of CiviCRM.                                    |
 |                                                                    |
 | CiviCRM is free software; you can copy, modify, and distribute it  |
 | under the terms of the GNU Affero General Public License           |
 | Version 3, 19 November 2007 and the CiviCRM Licensing Exception.   |
 |                                                                    |
 | CiviCRM is distributed in the hope that it will be useful, but     |
 | WITHOUT ANY WARRANTY; without even the implied warranty of         |
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               |
 | See the GNU Affero General Public License for more details.        |
 |                                                                    |
 | You should have received a copy of the GNU Affero General Public   |
 | License and the CiviCRM Licensing Exception along                  |
 | with this program; if not, contact CiviCRM LLC                     |
 | at info[AT]civicrm[DOT]org. If you have questions about the        |
 | GNU Affero General Public License or the licensing of CiviCRM,     |
 | see the CiviCRM license FAQ at http://civicrm.org/licensing        |
 +--------------------------------------------------------------------+
*/


/*
 * Developed by Jim Taylor.
 * CiviCRM Views Integration
 *
 * civicrm_handler_filter_activity_status.inc
 * Filters Activity Statuses, and is configured using the labels from the database
 *
 */
class civicrm_handler_filter_pseudo_constant extends views_handler_filter_in_operator {


/**
   * @var views_many_to_one_helper
   *
   * Stores the Helper object which handles the many_to_one complexity.
   */
  var $helper = NULL;

  function init(&$view, &$options) {
    parent::init($view, $options);
    $this->helper = new views_many_to_one_helper($this);
  }

  function option_definition() {
    $options = parent::option_definition();

    $options['operator']['default'] = 'or';
    $options['value']['default'] = array();

    if (isset($this->helper)) {
      $this->helper->option_definition($options);
    }
    else {
      $helper = new views_many_to_one_helper($this);
      $helper->option_definition($options);
    }

    return $options;
  }


  public $_pseudo_constant; function construct() {
    parent::construct();
    if (!civicrm_initialize() ||
      !isset($this->definition['pseudo class']) ||
      !isset($this->definition['pseudo method'])
    ) {
      return;
    }



    // Load pseudo arguments if passed as array from hook_views_data
    if (isset($this->definition['pseudo args']) && is_array($this->definition['pseudo args'])) {
      $pseudo_args = $this->definition['pseudo args'];
    }
    elseif (isset($this->definition['dao class']) && isset($this->definition['real field'])) {
      $pseudo_args = array($this->definition['dao class'], $this->definition['real field']);
    }
    else {
      $pseudo_args = array();
    }

    require_once str_replace('_', DIRECTORY_SEPARATOR, $this->definition['pseudo class']) . '.php';
    // Include and call the Pseudo Class method
    $this->_pseudo_constant = call_user_func_array($this->definition['pseudo class'] . "::" . $this->definition['pseudo method'], $pseudo_args);
  }


 function operators() {
    $operators = array(
      'in' => array(
        'title' => t('Is one of'),
        'short' => t('in'),
        'short_single' => t('='),
        'method' => 'op_simple',
        'values' => 1,
      ),
      'not in' => array(
        'title' => t('Is not one of'),
        'short' => t('not in'),
        'short_single' => t('<>'),
        'method' => 'op_simple',
        'values' => 1,
      ),
      'and' => array(
        'title' => t('Is all of'),
        'short' => t('and'),
        'short_single' => t('='),
        'method' => 'op_helper',
        'values' => 1,
        'ensure_my_table' => 'helper',
      ),
    );

    // if the definition allows for the empty operator, add it.
    if (!empty($this->definition['allow empty'])) {
      $operators += array(
        'empty' => array(
          'title' => t('Is empty (NULL)'),
          'method' => 'op_empty',
          'short' => t('empty'),
          'values' => 0,
        ),
        'not empty' => array(
          'title' => t('Is not empty (NOT NULL)'),
          'method' => 'op_empty',
          'short' => t('not empty'),
          'values' => 0,
        ),
      );
    }

    return $operators;
  }

  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = $this->definition['title'];
      $options = array();
      foreach ($this->_pseudo_constant as $id => $name) {
        $options[$id] = $name;
      }
      $this->value_options = $options;
    }
  }


 /**
   * Override ensure_my_table so we can control how this joins in.
   * The operator actually has influence over joining.
   */
  function ensure_my_table() {
    // Defer to helper if the operator specifies it.
    $info = $this->operators();
    if (isset($info[$this->operator]['ensure_my_table']) && $info[$this->operator]['ensure_my_table'] == 'helper') {
      return $this->helper->ensure_my_table();
    }

    return parent::ensure_my_table();
  }

  function op_helper() {
    if (empty($this->value)) {
      return;
    }
    $this->helper->add_filter();
  }
}

