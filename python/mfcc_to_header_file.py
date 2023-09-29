
class Header_writer:

   def digit_to_text(self, digit):
     if digit == 0: return 'zero'
     if digit == 1: return 'one'
     if digit == 2: return 'two'
     if digit == 3: return 'three'
     if digit == 4: return 'four'
     if digit == 5: return 'five'
     if digit == 6: return 'six'
     if digit == 7: return 'seven'
     if digit == 8: return 'eight'
     if digit == 9: return 'nine'
     return 'whoops'

   def __init__(self, filename):
     self.file = open(filename, 'w')

   def write_mfcc(self, mfcc, digit, stride = 6):
     rows = mfcc.shape[0]
     cols = mfcc.shape[1]

     self.file.write(' \n')
     self.file.write(' \n')
     self.file.write(' static const float {:s} [{:d}][{:d}] = {{ \n'.format('mfcc_' + self.digit_to_text(digit) + '_values', rows, cols));
     for row in range(rows):
       col = 0;
       while col < cols:
         if col==0:
           self.file.write('          {  ');
         else:
           self.file.write('             ');
         for s in range(stride):
           if ((col+s) < (cols-1)):
             self.file.write('{:13.8f}, '.format(mfcc[row][col+s]))
           if ((col+s) == (cols-1)):
             self.file.write('{:13.8f}  '.format(mfcc[row][col+s]))
         col += stride
         self.file.write(' \n')
       if row < rows-1:
         self.file.write('          },  \n')
       if row == rows-1:
         self.file.write('          }   \n')
          
     self.file.write('   }; \n')
     self.file.write(' \n')
       

   def close(self):
     self.file.close()

