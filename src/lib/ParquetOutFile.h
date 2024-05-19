#include <protocol/TCompactProtocol.h>
#include <transport/TBufferTransports.h>

#include "parquet/parquet_types.h"

namespace nanoparquet {

class ParquetOutFile {
public:
  ParquetOutFile(
    std::string filename,
    parquet::format::CompressionCodec::type codec
  );
  void set_num_rows(uint32_t nr);
  void schema_add_column(std::string name, parquet::format::Type::type type);
  void schema_add_column(std::string name,
                         parquet::format::LogicalType logical_type,
                         bool dict = false);
  void add_key_value_metadata(std::string key, std::string value);
  void write();

  // write out various parquet types, these must be implemented in
  // the subclass
  virtual void write_int32(std::ostream &file, uint32_t idx) = 0;
  virtual void write_double(std::ostream &file, uint32_t idx) = 0;
  virtual void write_byte_array(std::ostream &file, uint32_t idx) = 0;
  virtual void write_boolean(std::ostream &file, uint32_t idx) = 0;
  virtual uint32_t get_size_byte_array(uint32_t idx) = 0;

  // callbacks to write a byte array dictionary
  virtual bool has_byte_array_dictionary() { return false; }
  virtual uint32_t get_num_values_byte_array_dictionary(uint32_t idx) {
    throw std::runtime_error("Not implemented");
  }
  virtual uint32_t get_size_byte_array_dictionary(uint32_t idx) {
    throw std::runtime_error("Not implemented");
  }
  // Needs to write indices as int32_t
  virtual void write_byte_array_dictionary(std::ostream &file, uint32_t idx) {
    throw std::runtime_error("Not implemented");
  }
  virtual void write_dictionary_indices(std::ostream &file, uint32_t idx) {
    throw std::runtime_error("Not implemented");
  }

private:
  std::ofstream pfile;
  uint32_t num_rows, num_cols;
  bool num_rows_set;
  uint32_t total_size; // for the single row group we have for now
  parquet::format::CompressionCodec::type codec;

  std::vector<parquet::format::Encoding::type> encodings;
  std::vector<parquet::format::SchemaElement> schemas;
  std::vector<parquet::format::ColumnMetaData> column_meta_data;
  std::vector<parquet::format::KeyValue> kv;

  std::shared_ptr<apache::thrift::transport::TMemoryBuffer> mem_buffer;
  apache::thrift::protocol::TCompactProtocolFactoryT<
      apache::thrift::transport::TMemoryBuffer>
      tproto_factory;
  std::shared_ptr<apache::thrift::protocol::TProtocol> tproto;

  void write_columns();
  void write_column(uint32_t idx);
  void write_column_uncompressed_plain(uint32_t idx);
  void write_column_compressed_plain(uint32_t idx);
  void write_column_dictionary(uint32_t idx);
  void write_footer();

  ByteBuffer buf_unc;
  ByteBuffer buf_com;

  // internal utility functions
  parquet::format::ConvertedType::type get_converted_type_from_logical_type(
      parquet::format::LogicalType logical_type);
  parquet::format::Type::type
  get_type_from_logical_type(parquet::format::LogicalType logical_type);
  uint32_t calculate_column_data_size(uint32_t idx);
};

} // namespace nanoparquet
