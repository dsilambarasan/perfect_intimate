<?php

/**
 * @file
 *  Contains base migration classes used by other Commerce Kickstart modules.
 */

/**
 * Common features for all the migrations.
 */
abstract class CommerceKickstartMigration extends Migration {

  public function processImport(array $options = array()) {
    parent::processImport($options);
    // Do not force menu rebuilding. Otherwise pathauto will try to rebuild
    // in each node_insert invocation.
    variable_set('menu_rebuild_needed', FALSE);
  }
}

class CommerceKickstartPages extends CommerceKickstartMigration {

  public function __construct() {
    parent::__construct();
    $this->description = t('Import pages nodes.');

    // Create a map object for tracking the relationships between source rows
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'title' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
        ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Create a MigrateSource object.
    $this->source = new MigrateSourceCSV(drupal_get_path('module', 'commerce_kickstart_migrate') . '/import/pages.csv', $this->csvcolumns(), array('header_rows' => 1));

    $this->destination = new MigrateDestinationNode('page');

    $this->addFieldMapping('nid', 'nid');
    $this->addFieldMapping('title', 'title');
    $this->addFieldMapping('path', 'path');
    $this->addFieldMapping('body', 'body');
    $this->addFieldMapping('body:summary', 'excerpt');

    $this->addFieldMapping('uid', 'uid')->defaultValue(1);
    $this->addFieldMapping('is_new', 'is_new')->defaultValue(TRUE);
  }

  function csvcolumns() {
    // "nid", "Title", "Path", "Body"
    $columns[0] = array('nid', 'nid');
    $columns[1] = array('title', 'Title');
    $columns[2] = array('path', 'Path');
    $columns[3] = array('body', 'Body');
    return $columns;
  }
}

class CommerceKickstartCategories extends Migration{

  public function __construct(){
    parent::__construct(MigrateGroup::getInstance("Import Taxonmy Term"));

    $this->description = t("Import list of categories as Taxonmy");

    $this->columns = array(
      0 => array('parent_item_category_uid',t('Term id')),
      1 => array('parent_item_category_id',t('Term name')),
      2 => array('parent_item_category_desc',t('Term description')),
    );

    $this->source = new MigrateSourceCSV(DRUPAL_ROOT . '/import/NalpacCategoriesQuoted.csv', 
      $this->columns,
      array(
        'delimiter' => ',',
        'header_rows' => 1
      )
    );

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'parent_item_category_uid' => array(
          'type' => 'int',
          'not null'  => TRUE,
          'description' => 'Term id',
          'alias' => 'import',
        )
      ),
       MigrateDestinationTerm::getKeySchema()
    );

    $term_options['allow_duplicate_terms'] = FALSE;
    $this->destination = new MigrateDestinationTerm('product_category', $term_options);

    $this->addFieldMapping('name','parent_item_category_id');
    $this->addFieldMapping('description','parent_item_category_desc');
  }

  public function prepare($node, stdClass $row) {
    if ($node->name == 'ROOT') {
      return FALSE;
    }
    if ($parent = strstr($node->name, '_', true)) {
      $parent_term = current(taxonomy_get_term_by_name($parent, strtolower($node->vocabulary_machine_name)));
      if (!empty($parent_term)) {
        $node->parent = array($parent_term->tid);
      }
    }
  }

}


class CommerceKickstartChildCategories extends Migration{

  public function __construct(){
    parent::__construct(MigrateGroup::getInstance("Import Child Taxonmy Term"));

    $this->description = t("Import list of child categories as Taxonmy");

    $this->columns = array(
      0 => array('parent_item_category_uid',t('Parent Term id')),
      1 => array('parent_item_category_id',t('Parent Term name')),
      2 => array('parent_item_category_desc',t('Parent Term description')),
      3 => array('item_category_uid',t('Term description')),
      4 => array('item_category_id',t('Term description')),
      5 => array('item_category_desc',t('Term description')),
    );

    $this->source = new MigrateSourceCSV(DRUPAL_ROOT . '/import/NalpacCategoriesQuoted.csv', 
      $this->columns,
      array(
        'delimiter' => ',',
        'header_rows' => 1
      )
    );

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'item_category_uid' => array(
          'type' => 'int',
          'not null'  => TRUE,
          'description' => 'Term id',
          'alias' => 'import',
        )
      ),
       MigrateDestinationTerm::getKeySchema()
    );

    $term_options['allow_duplicate_terms'] = FALSE;
    $this->destination = new MigrateDestinationTerm('product_category', $term_options);

    $this->addFieldMapping('name','item_category_id');
    $this->addFieldMapping('description','item_category_desc');
  }

  public function prepare($node, stdClass $row) {
    //print_r($node);
    //print_r($row); exit;
    $parent_term = current(taxonomy_get_term_by_name($node->name, strtolower($node->vocabulary_machine_name)));
    if (!empty($parent_term)) {
      $taxonomy = current(taxonomy_get_term_by_name($row->parent_item_category_id, strtolower($node->vocabulary_machine_name)));
      if (!empty($taxonomy)) {
        $node->parent = array($taxonomy->tid);
      }
    }
    else {
      $taxonomy = current(taxonomy_get_term_by_name($row->parent_item_category_id, strtolower($node->vocabulary_machine_name)));
      if (!empty($taxonomy)) {
        $node->parent = array($taxonomy->tid);
      }
    }
  }

}
/*
class CommerceKickstartManufacturer extends Migration{

  public function __construct(){
    parent::__construct(MigrateGroup::getInstance("Import Manufacturers"));

    $this->description = t("Import list of Manufacturers");

    $this->columns = array(
      0 => array('Nalpac_Item_Number',t('Id')),
      1 => array('Description',t('Title')),
      2 => array('weight',t('Weight')),
      3 => array('MFG_Number',t('Manufacturer Number')),
      4 => array('MFG_Name',t('Manufacturer Name')),
      5 => array('WholeSale_Price',t('Price')),
      6 => array('Selling_UOM',t('Unit of Measure')),
      7 => array('Long_Description',t('Long_Description')),
      8 => array('extended_desc',t('Extended desc')),
      9 => array('Quantity_Onhand',t('Invertory')),
      10 => array('UPC_Number',t('UPC Number')),
      11 => array('Category_ID',t('Category')),
      12 => array('Category_Description',t('Category Description')),
    );

    $this->source = new MigrateSourceCSV(DRUPAL_ROOT . '/import/NalpacItems.csv', 
      $this->columns,
      array(
        'delimiter' => ',',
        'header_rows' => 1
      )
    );

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'Nalpac_Item_Number' => array(
          'type' => 'text',
          'not null'  => TRUE,
          'description' => 'Term id',
          'alias' => 'import',
        )
      ),
      MigrateDestinationTerm::getKeySchema()
    );

    $term_options['allow_duplicate_terms'] = TRUE;
    $this->destination = new MigrateDestinationTerm('Manufacturer', $term_options);

    $this->addFieldMapping('name','MFG_Name');
    $this->addFieldMapping('field_mfg_id','MFG_Number');

    // Unmapped destination fields
    $this->addUnmigratedSources(array(
      'Nalpac_Item_Number',
      'Description',
      'weight',
      'WholeSale_Price',
      'Selling_UOM',
      'Long_Description',
      'extended_desc',
      'Quantity_Onhand',
      'UPC_Number',
      'Category_ID',
      'Category_Description',
    ));
  }

}*/

class CommerceKickstartProductContent extends Migration{

  public function __construct(){
    parent::__construct(MigrateGroup::getInstance("Import content"));

    $this->description = t("Import list of content");

    $this->columns = array(
      0 => array('Nalpac_Item_Number',t('Id')),
      1 => array('Description',t('Title')),
      2 => array('weight',t('Weight')),
      3 => array('MFG_Number',t('Manufacturer Number')),
      4 => array('MFG_Name',t('Manufacturer Name')),
      5 => array('WholeSale_Price',t('Price')),
      6 => array('Selling_UOM',t('Unit of Measure')),
      7 => array('Long_Description',t('Long_Description')),
      8 => array('extended_desc',t('Extended desc')),
      9 => array('Quantity_Onhand',t('Invertory')),
      10 => array('UPC_Number',t('UPC Number')),
      11 => array('Category_ID',t('Category')),
      12 => array('Category_Description',t('Category Description')),
    );

    $this->source = new MigrateSourceCSV(DRUPAL_ROOT . '/import/NalpacItems_sample.csv', 
      $this->columns,
      array(
        'delimiter' => ',',
        'header_rows' => 1
      )
    );

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'Nalpac_Item_Number' => array(
          'type' => 'int',
          'length' => 10,
          'not null' => TRUE,
          'description' => 'UPC Number',
        ),
      ),
      MigrateDestinationEntityAPI::getKeySchema('commerce_product', 'product')
    );

    $this->destination = new MigrateDestinationEntityAPI('commerce_product', 'product');

    $this->addFieldMapping('title','Description');
    $this->addFieldMapping('uid')->defaultValue(1);
    $this->addFieldMapping('language')->defaultValue('und');

    $this->addFieldMapping('sku','UPC_Number');

    $this->addFieldMapping('field_weight','weight');
    $this->addFieldMapping('commerce_price','WholeSale_Price');
    $this->addFieldMapping('field_manufacturer_number','MFG_Number');
    $this->addFieldMapping('field_manufacturer','MFG_Name');
    $this->addFieldMapping('field_quantity','Quantity_Onhand');
    $this->addFieldMapping('field_selling_uom','Selling_UOM')->defaultValue('EA');
    $this->addFieldMapping('status')->defaultValue(1);


    // We will get the image data from a related table in prepareRow()
    $this->addFieldMapping('field_images', 'images');
    $this->addFieldMapping('field_images:destination_file', 'images_dest');
    // Indicate that we want each file to maintain its name, replacing any
    // previous file of the same name (as opposed to being renamed to avoid
    // collisions, which is the default).
    $this->addFieldMapping('field_images:file_replace')
         ->defaultValue(FILE_EXISTS_REPLACE);
    $this->addFieldMapping('field_images:alt', 'images_alt');
    $this->addFieldMapping('field_images:title', 'images_alt');
    $this->addFieldMapping('field_images:source_dir')->defaultValue('sampleimages');
    $this->addFieldMapping('field_images:preserve_files')->defaultValue(TRUE);

  }

  public function prepareRow($current_row) {
    $current_row->images[]  = str_pad($current_row->Nalpac_Item_Number, 4, '0', STR_PAD_LEFT) . '.jpg';
    $current_row->images_dest[]  = str_pad($current_row->Nalpac_Item_Number, 4, '0', STR_PAD_LEFT) . '.jpg';
    $current_row->images_alt[] = $current_row->Description;
    $current_row->images_src[] = 'sampleimages';
  }

}


class CommerceKickstartNodeContent extends Migration{

  public function __construct(){
    parent::__construct(MigrateGroup::getInstance("Import content"));
    $this->dependencies = array('CommerceKickstartProductContent');

    $this->description = t("Import list of content");

    $this->columns = array(
      0 => array('Nalpac_Item_Number',t('Id')),
      1 => array('Description',t('Title')),
      2 => array('weight',t('Weight')),
      3 => array('MFG_Number',t('Manufacturer Number')),
      4 => array('MFG_Name',t('Manufacturer Name')),
      5 => array('WholeSale_Price',t('Price')),
      6 => array('Selling_UOM',t('Unit of Measure')),
      7 => array('Long_Description',t('Long_Description')),
      8 => array('extended_desc',t('Extended desc')),
      9 => array('Quantity_Onhand',t('Invertory')),
      10 => array('UPC_Number',t('UPC Number')),
      11 => array('Category_ID',t('Category')),
      12 => array('Category_Description',t('Category Description')),
    );

    $this->source = new MigrateSourceCSV(DRUPAL_ROOT . '/import/NalpacItems_sample.csv', 
      $this->columns,
      array(
        'delimiter' => ',',
        'header_rows' => 1
      )
    );

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'Nalpac_Item_Number' => array(
          'type' => 'int',
          'length' => 10,
          'not null' => TRUE,
          'description' => 'UPC Number',
        ),
      ),
       MigrateDestinationNode::getKeySchema()
    );

    $this->destination = new MigrateDestinationEntityAPI('commerce_product', 'product');
    $this->destination = new MigrateDestinationNode('product_display');

    $this->addFieldMapping('title','Description');
    $this->addFieldMapping('body','Long_Description')->arguments(array('format' => 'full_html'));
    $this->addFieldMapping('body:summary', 'extended_desc');

    $this->addFieldMapping('field_product', 'Nalpac_Item_Number')->sourceMigration('CommerceKickstartProductContent');

    $this->addFieldMapping('field_product_category', 'Category_ID')->defaultValue(555);

  }

  public function prepare($node, stdClass $row) {
    if (!is_array($node->field_product_category)) {
      $term = current(taxonomy_get_term_by_name($node->field_product_category, 'product_category'));
      $node->field_product_category[LANGUAGE_NONE] = array($term->tid);
    }
  }

}
