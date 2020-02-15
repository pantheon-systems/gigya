<?php
/**
 * Created by PhpStorm.
 * User: Yaniv Aran-Shamir
 * Date: 7/7/16
 * Time: 9:37 AM
 */

class Gigya_DsQueryObject {

  const VALUE_REG_EXP = '/.*(and|or|where)\s.*/i';
  /**
   * @var string
   */
  private $query;
  /**
   * @var array
   */
  private $fields;
  /**
   * @var string
   */
  private $table;
  /**
   * @var array
   */
  private $ors;
  /**
   * @var array
   */
  private $ands;
  /**
   * @var string
   */
  private $oid;
  /**
   * @var array
   */
  private $operators;
  /**
   * @var array
   */
  private $ins;
  /**
   * @var string
   */
  /**
   * @var string
   */
  private $uid = NULL;

  /**
   * DsQueryObject constructor.
   *
   * @param gigyaCMS $gigyaCMS
   *
   */
  public function __construct() {
    $this->operators = array(
      "<",
      ">",
      "=",
      ">=",
      "<=",
      "!=",
      "contains",
      "not contains"
    );
  }

  public function addWhere($field, $op, $value, $valueType = "string") {
    $this->addAnd($field, $op, $value, $valueType);
    return $this;

  }

  /**
   * @param string $filed
   * @param array $terms
   *
   * @param  string $andOr
   *
   * @return $this
   */
  public function addIn($filed, $terms, $andOr = "and") {

    array_walk(
      $terms, function (&$term, &$key) {
      $term = '"' . trim($term, '"') . '"';
    }
    );
    $ins = join(', ', $terms);
    $in  = " in($ins)";
    if ("or" == $andOr) {
      $this->ors[] = 'data.' . $filed . $in;
    }
    else {
      $this->ands[] = 'data.' . $filed . $in;
    }

    return $this;
  }

  /**
   * @param string $field
   * @param string $term
   * @param string $andOr
   *
   * @return $this
   */
  public function addContains($field, $term, $andOr = "and") {
    if ("or" == $andOr) {
      $this->addOr($field, "contains", $term);
    }
    else {
      $this->addAnd($field, "contains", $term);
    }

    return $this;
  }

  /**
   * @param string $field
   * @param string $op
   * @param string $value
   *
   * @return $this
   * @throws Gigya_Ds_Helper_Gdo_DsQueryException
   */
  public function addOr($field, $op, $value, $valueType = "string") {
    $this->ors[] = $this->sanitizeAndBuild($field, $op, $value, $valueType);

    return $this;
  }

  private function sanitizeAndBuild(
    $field,
    $op,
    $value,
    $valueType = "string"
  ) {
    if (preg_match(self::VALUE_REG_EXP, $value)) {
      throw new Exception("bad value string");
    }
    if ("string" == $valueType) {
      $value = '"' . trim($value, '"') . '"';
    }
    elseif ("bool" == $valueType) {
      $value = ($value) ? 'true' : 'false';
    }
    if (empty($field) || empty($op) || strlen(trim($value)) === 0) {
      throw new Exception(
        "parameters can not be empty or a bad value string"
      );
    }
    if (!in_array($op, $this->operators)) {
      throw new Exception($op . " is not a valid operator");
    }

    return $this->prefixField(filter_var($field,
      FILTER_SANITIZE_STRING)) . " " . $op . " " . $value;
  }

  /**
   * @param string $field
   * @param string $op
   * @param string $value
   *
   * @return $this
   */
  public function addAnd($field, $op, $value, $valueType = "string") {
    $this->ands[] = $this->sanitizeAndBuild($field, $op, $value, $valueType);

    return $this;
  }

  /**
   * @param string $field
   * @param string $term
   * @param string $andOr
   *
   * @return $this
   */
  public function addNotContains($field, $term, $andOr = "and") {
    if ("or" == $andOr) {
      $this->addOr($field, "not contains", $term);
    }
    else {
      $this->addAnd($field, "not contains", $term);
    }

    return $this;
  }

  /**
   * @param string $field
   *
   * @return $this
   */
  public function addField($field) {
    $this->fields[] = $this->prefixField($field);

    return $this;
  }

  /**
   * @param string $field
   * @param string $andOr
   *
   * @return $this
   */
  public function addIsNull($field, $andOr = "and") {
    if ("and" == strtolower($andOr)) {
      $this->ands[] = $this->prefixField(trim($field)) . " is null";
    }
    elseif ("or" == strtolower($andOr)) {
      $this->ors[] = $this->prefixField(trim($field)) . " is null";
    }
    else {
      throw new Exception(
        'andOr parameter should "and" or "or"'
      );
    }

    return $this;

  }

  /**
   * @param string $field
   * @param string $andOr
   *
   * @return $this
   */
  public function addIsNotNull($field, $andOr = "and") {
    if ("and" == strtolower($andOr)) {
      $this->ands[] = $this->prefixField(trim($field)) . " is not null";
    }
    elseif ("or" == strtolower($andOr)) {
      $this->ors[] = $this->prefixField(trim($field)) . " is not null";
    }
    else {
      throw new Exception(
        'andOr parameter should "and" or "or"'
      );
    }

    return $this;

  }

  /**
   * @return mixed
   */
  public function getQuery() {
    return $this->query;
  }

  /**
   * @param mixed $query
   *
   * @return $this
   */
  public function setQuery($query) {
    $this->query = $query;

    return $this;
  }

  /**
   * @return mixed
   */
  public function getFields() {
    return $this->fields;
  }


  // getters and setters

  /**
   * @param mixed $fields
   *
   * @return $this
   */
  public function setFields($fields) {
    $this->fields = $fields;

    return $this;
  }

  /**
   * @return mixed
   */
  public function getTable() {
    return $this->table;
  }

  /**
   * @param mixed $table
   *
   * @return $this
   */
  public function setTable($table) {
    $this->table = $table;

    return $this;
  }

  /**
   * @return string mixed
   */
  public function getOid() {
    return $this->oid;
  }

  /**
   * @param string $oid
   *
   * @return $this
   */
  public function setOid($oid) {
    $this->oid = $oid;

    return $this;
  }

  /**
   * @return string
   */
  public function getUid() {
    return $this->uid;
  }

  /**
   * @param string $uid
   */
  public function setUid($uid) {
    $this->uid = $uid;
  }

  public function dsGet() {
    $paramsArray = array("oid" => $this->oid, "type" => $this->table);
    if (!empty($this->uid)) {
      $paramsArray['UID'] = $this->uid;
    }
    if (count($this->fields) > 0) {
      $paramsArray['fields'] = $this->buildFieldsStringForGet();
    }
    $res = _gigya_api("ds.get", $paramsArray);
    if (is_numeric($res)) {
      throw new Exception("Error getting info from DS", $res);
    }
    return $res['data'];
  }

  private function prefixField($field) {
    return 'uid' == strtolower($field) ? $field : 'data.' . $field;
  }

  private function buildFieldsString() {
    if (in_array("*", $this->fields)) {
      return '*';
    }
    $queryFields = array();
    foreach ($this->fields as $field) {
      $queryFields[] = $this->prefixField($field);
    }
    return join(", ", $queryFields);
  }
  private function buildFieldsStringForGet() {
    return in_array("*", $this->fields)
      ? "*"
      : join(
        ",", $this->fields
      );

  }

  public function dsSearch() {
    if (empty($this->query)) {
      $this->buildQuery();
    }

    $res = _gigya_api("ds.search", array("query" => $this->query));
    if (is_numeric($res)) {
      throw new Exception("Error getting info from DS", $res);
    }
    return $res['results'];
  }

  protected function buildQuery() {
    if (!$this->checkAllRequired()) {
      throw new Exception("missing fields or table");
    }
    $fields = $this->buildFieldsString();
    $q      = "SELECT " . $fields . " FROM " . $this->table;
    $where  = TRUE;
    if (!empty($this->ands)) {
      if ($where) {
        $q .= " WHERE ";
        $where = FALSE;
      }
      else {
        $q .= " AND ";
      }
      $ands = join(" AND ", $this->ands);
      $q .= $ands;
    }
    if (!empty($this->ors)) {
      if ($where) {
        $q .= " WHERE ";
        $where = FALSE;
      }
      else {
        $q .= " OR ";
      }
      $ors = join(" OR ", $this->ors);
      $q .= $ors;
    }
    $this->query = $q;
  }

  /**
   * @return bool
   */
  private function checkAllRequired() {
    return !(empty($this->fields) && empty($this->table));
  }

  public function dsDelete() {

    $paramsArray = array("oid" => $this->oid, "type" => $this->table);
    if (!empty($this->uid)) {
      $paramsArray['UID'] = $this->uid;
    }
    if (count($this->fields) > 0) {
      $paramsArray['fields'] = $this->buildFieldsString();
    }
    $res = _gigya_api("ds.delete", $paramsArray);
    if (is_numeric($res)) {
      throw new Exception("Error deleting " . $this->oid . " from DS", $res);
    }
  }

}