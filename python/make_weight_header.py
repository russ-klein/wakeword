import keyword_model
import write_weights

#create inference model
model = keyword_model.keyword_nopad_model()

#you can train it, but to save time, load a known good set of weights
model.load_weights('small_weights')

write_weights.write_header_file(model)

